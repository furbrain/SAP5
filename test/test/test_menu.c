#include "unity.h"
#include "menu.h"
#include "exception.h"

void action_func(int x);
char long_str[] = "Really ridiculously long string that should break things";

DECLARE_MENU(subsubtest, {
    {"Lots", Info, {NULL}, 0},
    {"Back", Back, {NULL}, 0}
    });

DECLARE_MENU(subtest, {
    {"Ten", Info, {NULL}, 0},
    {"Eleven", Action, {action_func}, 11},
    {"Twelve", SubMenu, {.submenu=&subsubtest}, 0},
    {"Thirteen", Back, {NULL}, 0}
    });

DECLARE_MENU(test, {
    {"One", Info, {NULL}, 0},
    {"Two", Action, {action_func}, 2},
    {"Three", SubMenu, {.submenu=&subtest}, 3}
    });
    
DECLARE_EMPTY_MENU(dynamic, 2);

int test_data;
void setUp(void)
{
    test_data = 0;
}

void tearDown(void)
{
}

void reset_menus(void) {
    test.current_entry = 0;
    subtest.current_entry = 0;
    subsubtest.current_entry = 0;
    test.submenu = NULL;
    subtest.submenu = NULL;
    subsubtest.submenu = NULL;
    dynamic.length = 0;
    dynamic.submenu = NULL;
    dynamic.current_entry = 0;
}

void action_func(int x) {
    test_data = x;
}


void test_declare_menu(void) {
    TEST_ASSERT_EQUAL_STRING("One", test.entries[0].text);
    TEST_ASSERT_EQUAL_STRING("Two", test.entries[1].text);
    TEST_ASSERT_EQUAL_STRING("Three", test.entries[2].text);
    TEST_ASSERT_EQUAL(Info, test.entries[0].type);
    TEST_ASSERT_EQUAL(Action, test.entries[1].type);
    TEST_ASSERT_EQUAL(0, test.entries[0].argument);
    TEST_ASSERT_EQUAL(2, test.entries[1].argument);
    TEST_ASSERT_EQUAL(3, test.entries[2].argument);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(3, test.length);
    TEST_ASSERT_NULL(test.submenu);
}

/* empty a menu */
void test_menu_clear(void) {
    reset_menus();
    dynamic.length = 1;
    dynamic.submenu = &subtest;
    menu_clear(&dynamic);
    TEST_ASSERT_NULL(dynamic.submenu);
    TEST_ASSERT_EQUAL(0, dynamic.length);
}

/* add an info entry to a menu */
void test_menu_append_info(void) {
    reset_menus();
    menu_append_info(&dynamic,"Info");
    TEST_ASSERT_EQUAL(1, dynamic.length);
    TEST_ASSERT_EQUAL_STRING("Info",menu_get_text(&dynamic));
    TEST_ASSERT_EQUAL(Info, dynamic_entries[0].type);
    TEST_ASSERT_THROWS(menu_append_info(&dynamic, long_str), ERROR_STRING_TOO_BIG);
    menu_append_info(&dynamic,"Info2");
    TEST_ASSERT_THROWS(menu_append_info(&dynamic,"Info3"), ERROR_MENU_FULL);
}

/* add a submenu entry to a menu */
void test_menu_append_submenu(void) {
    reset_menus();
    menu_append_submenu(&dynamic,"sub1",&subtest);
    TEST_ASSERT_EQUAL(1, dynamic.length);
    TEST_ASSERT_EQUAL_STRING("sub1",menu_get_text(&dynamic));
    TEST_ASSERT_EQUAL(SubMenu, dynamic_entries[0].type);
    TEST_ASSERT_EQUAL_PTR(&subtest, dynamic_entries[0].submenu);
    TEST_ASSERT_THROWS(menu_append_submenu(&dynamic, long_str, &subtest), ERROR_STRING_TOO_BIG);
    menu_append_submenu(&dynamic,"sub1",&subtest);
    TEST_ASSERT_THROWS(menu_append_submenu(&dynamic,"sub1",&subtest), ERROR_MENU_FULL);
}

/* add an action entry to a menu */
void test_menu_append_action(void) {
    reset_menus();
    menu_append_action(&dynamic, "action1", action_func, 1);
    TEST_ASSERT_EQUAL(1, dynamic.length);
    TEST_ASSERT_EQUAL_STRING("action1", menu_get_text(&dynamic));
    TEST_ASSERT_EQUAL(Action, dynamic_entries[0].type);
    TEST_ASSERT_EQUAL(1, dynamic_entries[0].argument);
    TEST_ASSERT_EQUAL_PTR(action_func, dynamic_entries[0].action);
    TEST_ASSERT_THROWS(menu_append_action(&dynamic, long_str, action_func, 2), ERROR_STRING_TOO_BIG);
    menu_append_action(&dynamic, "action2", action_func, 2);
    TEST_ASSERT_THROWS(menu_append_action(&dynamic, "action3", action_func, 3), ERROR_MENU_FULL);
}

/* add a back entry to a menu */
void test_menu_append_back(void) {
    reset_menus();
    menu_append_back(&dynamic,"Back");
    TEST_ASSERT_EQUAL(1, dynamic.length);
    TEST_ASSERT_EQUAL_STRING("Back",menu_get_text(&dynamic));
    TEST_ASSERT_EQUAL(Back, dynamic_entries[0].type);
    TEST_ASSERT_THROWS(menu_append_back(&dynamic, long_str), ERROR_STRING_TOO_BIG);
    menu_append_back(&dynamic,"Back2");
    TEST_ASSERT_THROWS(menu_append_back(&dynamic,"Back3"), ERROR_MENU_FULL);
}

void test_menu_prev(void) {
    reset_menus();
    TEST_ASSERT_EQUAL(0, test.current_entry);
    menu_prev(&test);        
    TEST_ASSERT_EQUAL(2, test.current_entry);
    menu_prev(&test);        
    TEST_ASSERT_EQUAL(1, test.current_entry);
    menu_prev(&test);        
    TEST_ASSERT_EQUAL(0, test.current_entry);
}

void test_menu_prev_with_submenu(void) {
    reset_menus();
    test.submenu=&subtest;
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(0, subtest.current_entry);
    menu_prev(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(3, subtest.current_entry);
    menu_prev(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(2, subtest.current_entry);
    menu_prev(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(1, subtest.current_entry);
    menu_prev(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(0, subtest.current_entry);
}

void test_menu_next(void) {
    reset_menus();
    TEST_ASSERT_EQUAL(0, test.current_entry);
    menu_next(&test);        
    TEST_ASSERT_EQUAL(1, test.current_entry);
    menu_next(&test);        
    TEST_ASSERT_EQUAL(2, test.current_entry);
    menu_next(&test);        
    TEST_ASSERT_EQUAL(0, test.current_entry);
}

void test_menu_next_with_submenu(void) {
    reset_menus();
    test.submenu=&subtest;
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(0, subtest.current_entry);
    menu_next(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(1, subtest.current_entry);
    menu_next(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(2, subtest.current_entry);
    menu_next(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(3, subtest.current_entry);
    menu_next(&test);
    TEST_ASSERT_EQUAL(0, test.current_entry);
    TEST_ASSERT_EQUAL(0, subtest.current_entry);
}


void test_menu_get_text(void) {
    reset_menus();
    TEST_ASSERT_EQUAL_STRING("One", menu_get_text(&test));
    test.current_entry = 1;
    TEST_ASSERT_EQUAL_STRING("Two", menu_get_text(&test));
} 

void test_menu_get_text_with_submenu(void) {
    reset_menus();
    TEST_ASSERT_EQUAL_STRING("One", menu_get_text(&test));
    test.submenu = &subtest;
    TEST_ASSERT_EQUAL_STRING("Ten", menu_get_text(&test));
    test.current_entry = 1;
    TEST_ASSERT_EQUAL_STRING("Ten", menu_get_text(&test));
    subtest.current_entry = 1;
    TEST_ASSERT_EQUAL_STRING("Eleven", menu_get_text(&test));
    subtest.submenu = &subsubtest;
    TEST_ASSERT_EQUAL_STRING("Lots", menu_get_text(&test));
} 

void test_menu_initialise(void) {
    reset_menus();
    test.submenu = &subtest;
    test.current_entry = 1;
    menu_initialise(&test);
    TEST_ASSERT_NULL(test.submenu);
    TEST_ASSERT_EQUAL(0, test.current_entry);
}    

void test_menu_action_submenu(void) {
    enum action result;
    reset_menus();
    test.current_entry = 2;
    subtest.current_entry = 1;
    TEST_ASSERT_NULL(test.submenu);
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(SubMenu, result);
    TEST_ASSERT_EQUAL_PTR(&subtest, test.submenu);
    TEST_ASSERT_EQUAL(0, subtest.current_entry);
    subtest.current_entry = 2;
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(SubMenu, result);
    TEST_ASSERT_EQUAL_PTR(&subtest, test.submenu);
    TEST_ASSERT_EQUAL_PTR(&subsubtest, subtest.submenu);   
}

void test_menu_action_back(void) {
    enum action result;
    reset_menus();
    test.current_entry = 2;
    test.submenu = &subtest;
    subtest.current_entry = 2;
    subtest.submenu = &subsubtest;
    subsubtest.current_entry = 1;
    
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(Back, result);
    TEST_ASSERT_EQUAL_PTR(&subtest, test.submenu);
    subtest.current_entry = 3;
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(Back, result);
    TEST_ASSERT_NULL(subtest.submenu);
    TEST_ASSERT_NULL(test.submenu);
}

void test_menu_action_action(void) {
    enum action result;
    reset_menus();
    test.current_entry = 1;
    TEST_ASSERT_EQUAL(0, test_data);
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(Action, result);
    TEST_ASSERT_EQUAL(2, test_data);
}

void test_menu_action_action_with_submenu(void) {
    enum action result;
    reset_menus();
    test.submenu = &subtest;
    subtest.current_entry = 1;
    TEST_ASSERT_EQUAL(0, test_data);
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(Action, result);
    TEST_ASSERT_EQUAL(11, test_data);
}

void test_menu_action_info(void) {
    enum action result;
    reset_menus();
    test.current_entry = 0;
    result = menu_action(&test);
    TEST_ASSERT_EQUAL(Info, result);
}
