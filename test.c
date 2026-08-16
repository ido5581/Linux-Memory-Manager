#include "uapi_mm.h"
#include <stdio.h>
#include "mm.h"
void* xcalloc(char*, int);
void xfree(void*);
void* xrealloc(void*, size_t);
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

void test_xrealloc() {
    printf("=== Starting xrealloc Tests ===\n\n");
    mm_init(); 
    mm_instantiate_new_page_family("emp_t", sizeof(emp_t));
    printf("--- Test 1: Basic Allocation ---\n");
    
    emp_t* emp1 = (emp_t*)xcalloc("emp_t", 1); 
    if(!emp1) {
        printf("Allocation failed!\n");
        return;
    }
    strcpy(emp1->name, "Ido");
    printf("emp1 allocated at: %p\n", (void*)emp1);
    printf("emp1 Name: %s\n\n", emp1->name);
    printf("--- Test 2: In-Place Expansion ---\n");
    emp_t* emp2 = (emp_t*)xcalloc("emp_t", 1);
    emp_t* emp3 = (emp_t*)xcalloc("emp_t", 1);
    
    printf("Addresses:\n");
    printf("emp1: %p\n", (void*)emp1);
    printf("emp2: %p\n", (void*)emp2);
    printf("emp3: %p\n", (void*)emp3);
    
    printf("Freeing emp2 to create space after emp1...\n");
    xfree(emp2);
    
    printf("Reallocating emp1 to double size...\n");
    emp_t* expanded_emp1 = (emp_t*)xrealloc(emp1, sizeof(emp_t) * 2);
    printf("expanded_emp1 is at: %p (Expected: SAME as emp1)\n", (void*)expanded_emp1);
    printf("Data preserved? Name: %s\n\n", expanded_emp1->name);
    printf("--- Test 3: Relocation (Move to new address) ---\n");
    printf("Reallocating expanded_emp1 to size * 4...\n");
    emp_t* relocated_emp = (emp_t*)xrealloc(expanded_emp1, sizeof(emp_t) * 4);
    printf("relocated_emp is at: %p (Expected: DIFFERENT from expanded_emp1)\n", (void*)relocated_emp);
    printf("Data preserved? Name: %s\n\n", relocated_emp->name);
    printf("--- Test 4: Shrinking ---\n");
    printf("Shrinking relocated_emp back to single size...\n");
    emp_t* shrunk_emp = (emp_t*)xrealloc(relocated_emp, sizeof(emp_t));
    printf("shrunk_emp is at: %p (Expected: SAME as relocated_emp)\n", (void*)shrunk_emp);
    printf("Data preserved? Name: %s\n\n", shrunk_emp->name);

    printf("--- Test 5: Edge Case (Size == 0) ---\n");
    printf("Freeing shrunk_emp using xrealloc(ptr, 0)...\n");
    void* null_ptr = xrealloc(shrunk_emp, 0);
    printf("Returned pointer: %p (Expected: (nil))\n\n", null_ptr);
    
    xfree(emp3);
    printf("=== All Tests Completed! ===\n");
}

int main(int argc, char **argv){
    test_xrealloc();
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

