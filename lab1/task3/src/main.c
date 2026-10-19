#include <stdio.h>

#include "cli.h"
#include "list.h"
#include "workload.h"

int main(int argc, char* argv[]) {
   workload_t w;
   if (cli_parse(argc, argv, &w) != 0) {
      cli_usage(argv[0]);
      return 1;
   }

   list_t list;
   list_init(&list);
   long inserted = workload_fill(&list, w);
   printf("Inserted %ld keys in empty list\n", inserted);

   workload_result_t r;
   if (workload_run(&list, w, &r) != 0) {
      fprintf(stderr, "Not enough memory\n");
      list_free(&list);
      return 1;
   }

   printf("Lock           = %s\n", w.lock->name);
   printf("Threads        = %d\n", w.thread_count);
   printf("Elapsed time   = %e seconds\n", r.elapsed);
   printf("Total ops      = %ld\n", w.total_ops);
   printf("member ops     = %ld\n", r.member_ops);
   printf("insert ops     = %ld\n", r.insert_ops);
   printf("delete ops     = %ld\n", r.delete_ops);
   printf("List size      = %ld\n", list_size(&list));

   list_free(&list);
   return 0;
}
