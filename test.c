#include "uapi_mm.h"
#include <stdio.h>
#include "mm.h"
void* xcalloc(char*, int);
void xfree(void*);
void mm_print_memory_usage(char *struct_name);
typedef struct emp_{
    char name[30];
    uint32_t emp_id;

}emp_t;

typedef struct student_{
    char name[30];
    uint32_t rollno;
    uint32_t marks_phys;
    uint32_t marks_chem;
    uint32_t marks_maths;
    struct student_ *next;
}student_t;

int main(int argc, char **argv){
    mm_init();
    MM_REG_STRUCT(emp_t);
    MM_REG_STRUCT(student_t);
    lookup_page_family_by_name ("emp_t");

    emp_t* emp1 = (emp_t*)xcalloc("emp_t", 1);
    emp_t* emp2 = (emp_t*)xcalloc("emp_t", 1);
    emp_t* emp3 = (emp_t*)xcalloc("emp_t", 1);

    emp1->emp_id = 100;
    emp2->emp_id = 200;
    emp3->emp_id = 300;
    printf("-------------------- Before Free (emp2) --------------------------\n");
    mm_print_memory_usage("emp_t");
    xfree(emp2);

    printf("\n-------------------- After Free (emp2) --------------------------\n");
    mm_print_memory_usage("emp_t");
    
    return 0;
}

