/*
 * test_rwlock.c — проверка блокировок чтения-записи и списка под ними.
 * Каждый тест запускается и для библиотечной блокировки, и для своей.
 *
 * Запуск:  make test
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "rwlock_api.h"
#include "list.h"
#include "workload.h"
#include "timer.h"

static int failed = 0, total = 0;

/* ---------- сторож: зависание — тоже провал ----------
   Если блокировка написана неправильно, потоки часто не падают, а
   навсегда засыпают в ожидании друг друга (взаимная блокировка).
   Сторож ждёт каждый тест не дольше WATCHDOG_SEC секунд. */

#define WATCHDOG_SEC 20

static const char* volatile current_test = "";
static atomic_long          test_started;      /* секунды, когда начался тест */

static void begin(const char* name) {
   current_test = name;
   atomic_store(&test_started, (long) time(NULL));
}

static void* watchdog(void* arg) {
   (void) arg;
   for (;;) {
      sleep(1);
      if (time(NULL) - atomic_load(&test_started) > WATCHDOG_SEC) {
         printf("[FAIL] тест «%s» завис больше %d с: взаимная блокировка\n",
                current_test, WATCHDOG_SEC);
         _exit(1);
      }
   }
   return NULL;
}

static void check(int ok, const char* lock, const char* what) {
   total++;
   if (!ok) failed++;
   printf("[%s] %-3s  %s\n", ok ? " OK " : "FAIL", lock, what);
}

static void sleep_ms(long ms) {
   struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
   nanosleep(&ts, NULL);
}

/* имя не t: внутри макроса GET_TIME уже есть своя переменная t */
static double now(void) { double sec; GET_TIME(sec); return sec; }

/* ---------- 1. Читатели читают одновременно ---------- */

#define READERS 4

typedef struct {
   const rwlock_api_t* api;
   void*               lock;
   atomic_int          inside;     /* сколько потоков СЕЙЧАС внутри rdlock */
   atomic_int          all_met;    /* увидели ли все READERS друг друга    */
} readers_ctx_t;

static void* reader_meet(void* arg) {
   readers_ctx_t* c = arg;
   c->api->rdlock(c->lock);
   atomic_fetch_add(&c->inside, 1);
   /* Ждём остальных ВНУТРИ блокировки чтения. Если бы блокировка
      пускала читателей по одному, они бы здесь не встретились. */
   double deadline = now() + 2.0;
   while (atomic_load(&c->inside) < READERS && now() < deadline) {}
   if (atomic_load(&c->inside) == READERS) atomic_store(&c->all_met, 1);
   atomic_fetch_sub(&c->inside, 1);   /* вышел — больше не внутри */
   c->api->unlock(c->lock);
   return NULL;
}

static void test_readers_concurrent(const rwlock_api_t* api) {
   begin("читатели одновременно");
   readers_ctx_t c = { .api = api, .lock = api->create() };
   atomic_init(&c.inside, 0);
   atomic_init(&c.all_met, 0);
   pthread_t th[READERS];
   for (int i = 0; i < READERS; i++) pthread_create(&th[i], NULL, reader_meet, &c);
   for (int i = 0; i < READERS; i++) pthread_join(th[i], NULL);
   check(atomic_load(&c.all_met), api->name,
         "4 читателя одновременно внутри rdlock");
   api->destroy(c.lock);
}

/* Читатели, ждавшие писателя, после его unlock входят ВСЕ сразу,
   а не по одному (unlock должен будить всех читателей, не одного) */
static void test_waiting_readers_wake_together(const rwlock_api_t* api) {
   begin("ждавшие читатели входят вместе");
   readers_ctx_t c = { .api = api, .lock = api->create() };
   atomic_init(&c.inside, 0);
   atomic_init(&c.all_met, 0);

   api->wrlock(c.lock);                        /* главный поток пишет */
   pthread_t th[READERS];
   for (int i = 0; i < READERS; i++) pthread_create(&th[i], NULL, reader_meet, &c);
   sleep_ms(100);                              /* читатели встали в очередь */
   api->unlock(c.lock);
   for (int i = 0; i < READERS; i++) pthread_join(th[i], NULL);

   check(atomic_load(&c.all_met), api->name,
         "4 читателя, ждавших писателя, вошли одновременно");
   api->destroy(c.lock);
}

/* ---------- 2. Писатель ждёт, пока читатель не выйдет ---------- */

typedef struct {
   const rwlock_api_t* api;
   void*               lock;
   atomic_int          writer_in;
} wait_ctx_t;

static void* writer_try(void* arg) {
   wait_ctx_t* c = arg;
   c->api->wrlock(c->lock);
   atomic_store(&c->writer_in, 1);
   c->api->unlock(c->lock);
   return NULL;
}

static void test_writer_waits_for_reader(const rwlock_api_t* api) {
   begin("писатель ждёт читателя");
   wait_ctx_t c = { .api = api, .lock = api->create() };
   atomic_init(&c.writer_in, 0);

   api->rdlock(c.lock);                       /* главный поток читает */
   pthread_t th;
   pthread_create(&th, NULL, writer_try, &c);
   sleep_ms(100);
   int blocked = !atomic_load(&c.writer_in);  /* писатель должен ждать */
   api->unlock(c.lock);
   pthread_join(th, NULL);                    /* а теперь — войти */

   check(blocked && atomic_load(&c.writer_in), api->name,
         "wrlock ждёт, пока читатель не вызовет unlock");
   api->destroy(c.lock);
}

/* ---------- 3. Писатель работает один ---------- */

#define WRITES 20000

typedef struct {
   const rwlock_api_t* api;
   void*               lock;
   long                a, b;          /* инвариант: a == b вне блокировки */
   atomic_int          stop;
   atomic_long         violations;    /* сколько раз читатель увидел a != b */
} excl_ctx_t;

static void* excl_writer(void* arg) {
   excl_ctx_t* c = arg;
   for (int i = 0; i < WRITES; i++) {
      c->api->wrlock(c->lock);
      c->a++;
      for (volatile int k = 0; k < 50; k++) {}   /* «окно», где a != b */
      c->b++;
      c->api->unlock(c->lock);
   }
   return NULL;
}

static void* excl_reader(void* arg) {
   excl_ctx_t* c = arg;
   while (!atomic_load(&c->stop)) {
      c->api->rdlock(c->lock);
      if (c->a != c->b) atomic_fetch_add(&c->violations, 1);
      c->api->unlock(c->lock);
   }
   return NULL;
}

static void test_writer_exclusive(const rwlock_api_t* api) {
   begin("писатель работает один");
   excl_ctx_t c = { .api = api, .lock = api->create() };
   atomic_init(&c.stop, 0);
   atomic_init(&c.violations, 0);

   pthread_t w[2], r[2];
   for (int i = 0; i < 2; i++) pthread_create(&r[i], NULL, excl_reader, &c);
   for (int i = 0; i < 2; i++) pthread_create(&w[i], NULL, excl_writer, &c);
   for (int i = 0; i < 2; i++) pthread_join(w[i], NULL);
   atomic_store(&c.stop, 1);
   for (int i = 0; i < 2; i++) pthread_join(r[i], NULL);

   check(c.a == 2 * WRITES && c.b == 2 * WRITES, api->name,
         "2 писателя по 20000 раз: ни одно изменение не потеряно");
   check(atomic_load(&c.violations) == 0, api->name,
         "читатели ни разу не увидели недописанные данные");
   api->destroy(c.lock);
}

/* ---------- 4. Писатель не голодает при потоке читателей ---------- */

typedef struct {
   const rwlock_api_t* api;
   void*               lock;
   atomic_int          stop;
} starve_ctx_t;

static void* busy_reader(void* arg) {
   starve_ctx_t* c = arg;
   while (!atomic_load(&c->stop)) {
      c->api->rdlock(c->lock);
      sleep_ms(1);                  /* читатели перекрываются во времени */
      c->api->unlock(c->lock);
   }
   return NULL;
}

static void* stop_after_2s(void* arg) {
   starve_ctx_t* c = arg;
   double deadline = now() + 2.0;
   while (!atomic_load(&c->stop) && now() < deadline) sleep_ms(5);
   atomic_store(&c->stop, 1);
   return NULL;
}

static void test_writer_not_starved(const rwlock_api_t* api, int must_pass) {
   begin("писатель не голодает");
   starve_ctx_t c = { .api = api, .lock = api->create() };
   atomic_init(&c.stop, 0);

   pthread_t r[4], stopper;
   for (int i = 0; i < 4; i++) pthread_create(&r[i], NULL, busy_reader, &c);
   sleep_ms(50);
   pthread_create(&stopper, NULL, stop_after_2s, &c);

   double start = now();
   api->wrlock(c.lock);
   double waited = now() - start;
   api->unlock(c.lock);

   atomic_store(&c.stop, 1);
   pthread_join(stopper, NULL);
   for (int i = 0; i < 4; i++) pthread_join(r[i], NULL);
   api->destroy(c.lock);

   char what[256];
   snprintf(what, sizeof what,
            "писатель среди 4 активных читателей ждал %.3f с%s", waited,
            waited >= 1.9 ? " (голодал, пока читатели не остановились)" : "");
   if (must_pass) check(waited < 0.5, api->name, what);
   else           printf("[INFO] %-3s  %s\n", api->name, what);
}

/* ---------- 5. Список под нагрузкой остаётся корректным ---------- */

static void test_list_consistent(const rwlock_api_t* api) {
   begin("список под нагрузкой");
   workload_t w = { .thread_count = 4, .lock = api, .initial_inserts = 1000,
                    .total_ops = 20000, .search_percent = 0.5, .insert_percent = 0.25 };
   list_t list;
   list_init(&list);
   long start_size = workload_fill(&list, w);

   workload_result_t r;
   workload_run(&list, w, &r);

   int sorted = 1;
   for (list_node_t* n = list.head; n != NULL && n->next != NULL; n = n->next)
      if (n->data >= n->next->data) sorted = 0;

   check(sorted, api->name, "список после 20000 операций на 4 потоках упорядочен, без повторов");
   check(list_size(&list) == start_size + r.inserted - r.deleted, api->name,
         "размер списка = начальный + вставленные - удалённые");
   check(r.member_ops + r.insert_ops + r.delete_ops == w.total_ops, api->name,
         "выполнены все операции");
   list_free(&list);
}

int main(void) {
   const rwlock_api_t* apis[] = { &LIB_RWLOCK, &MY_RWLOCK };

   setvbuf(stdout, NULL, _IOLBF, 0);     /* печатать сразу, построчно */
   begin("");
   pthread_t dog;
   pthread_create(&dog, NULL, watchdog, NULL);
   pthread_detach(dog);

   for (int i = 0; i < 2; i++) {
      test_readers_concurrent(apis[i]);
      test_waiting_readers_wake_together(apis[i]);
      test_writer_waits_for_reader(apis[i]);
      test_writer_exclusive(apis[i]);
      test_list_consistent(apis[i]);
   }
   /* Приоритет писателей обещает только наша блокировка; для
      библиотечной это зависит от системы, поэтому просто показываем */
   test_writer_not_starved(&MY_RWLOCK, 1);
   test_writer_not_starved(&LIB_RWLOCK, 0);

   printf("\n%s: %d из %d тестов прошли\n",
          failed ? "ЕСТЬ ОШИБКИ" : "ВСЁ ВЕРНО", total - failed, total);
   return failed ? 1 : 0;
}
