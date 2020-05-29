#include <string>
#include "gtest/gtest.h"

#include "list.h"

extern "C" {
#include "log.mock.h"
#include "mem.mock.h"
}

class list_tests : public ::testing::Test
{
protected:
	list_tests() : list_element0{}, list_element1{}, list_element2{}, list_element3{}, list_element4{}, list_element5{},
	                a_valid("a_valid"), b_valid("b_valid"), c_valid("c_valid"),
	               d_valid("d_valid"), e_valid("e_valid"), f_valid("f_valid"), g_valid("g_valid")
    {
	}

	~list_tests() override = default;

	void SetUp() override
    {
        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);
        RESET_FAKE(r_err);
	}

	void TearDown() override
    {
	}

	_list_t *CreateList3(void *data0, void *data1, void *data2)
    {
        list_element0.next = &list_element1;
        list_element0.data = data0;

        list_element1.next = &list_element2;
        list_element1.data = data1;

        list_element2.next = nullptr;
        list_element2.data = data2;
        return &list_element0;
    }

    _list_t *CreateList6(void *data0, void *data1, void *data2, void *data3, void *data4, void *data5)
    {

	    _list_t *plist = CreateList3(data0, data1, data2);
        list_element2.next = &list_element3;

        list_element3.next = &list_element4;
        list_element3.data = data3;

        list_element4.next = &list_element5;
        list_element4.data = data4;

        list_element5.next = nullptr;
        list_element5.data = data5;

        return plist;
    }

    _list_t list_element0;
    _list_t list_element1;
    _list_t list_element2;
    _list_t list_element3;
    _list_t list_element4;
    _list_t list_element5;

    const char *a_valid;
    const char *b_valid;
    const char *c_valid;
    const char *d_valid;
    const char *e_valid;
    const char *f_valid;
    const char *g_valid;

};

TEST_F(list_tests, new_list_when_r_new_succeeds_returns_valid_pointer)
{
    // Arrange
    _list_t list{nullptr, nullptr};

    safe_malloc_fake.return_val = &list;

    // Act
    _list_t *plist = new_list(nullptr);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(plist, &list);
}

TEST_F(list_tests, new_list_when_r_new_succeeds_sets_data_next_nullptr)
{
    // Arrange
    _list_t list{nullptr, nullptr};;
    int data = 10;
    int expected_data = 10;

    safe_malloc_fake.return_val = &list;

    // Act
    _list_t *plist = new_list(&data);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(*(int*)plist->data, expected_data);
    ASSERT_EQ(plist->next, nullptr);
}

TEST_F(list_tests, delete_list_when_nullptr_pointer_passed_does_not_delete)
{
    // Arrange
    // Act
    delete_list(nullptr);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 0);
}

TEST_F(list_tests, delete_list_one_list_entry_deletes_correct_number)
{
    // Arrange
    _list_t list = {nullptr, nullptr};

    // Act
    delete_list(&list);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(list_tests, delete_list_multiple_list_entry_deletes_correct_number)
{
    // Arrange
    _list_t list3 = {nullptr, nullptr};
    _list_t list2 = {nullptr, &list3};
    _list_t list1 = {nullptr, &list2};

    // Act
    delete_list(&list1);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 3);
    ASSERT_EQ(safe_free_fake.arg0_history[0], &list1);
    ASSERT_EQ(safe_free_fake.arg0_history[1], &list2);
    ASSERT_EQ(safe_free_fake.arg0_history[2], &list3);
}

TEST_F(list_tests, delete_list_and_data_when_data_null_does_not_delete_data)
{
    // Arrange
    _list_t list3 = {nullptr, nullptr};
    _list_t list2 = {nullptr, &list3};
    _list_t list1 = {nullptr, &list2};

    // Act
    delete_list_and_data(&list1, NULL);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 3);
    ASSERT_EQ(safe_free_fake.arg0_history[0], &list1);
    ASSERT_EQ(safe_free_fake.arg0_history[1], &list2);
    ASSERT_EQ(safe_free_fake.arg0_history[2], &list3);
}

TEST_F(list_tests, delete_list_and_data_when_data_not_null_deletes_data)
{
    // Arrange
     int d1 = 1;
     int d3 = 3;
    _list_t list3 = {&d3, nullptr};
    _list_t list2 = {nullptr, &list3};
    _list_t list1 = {&d1, &list2};

    // Act
    delete_list_and_data(&list1, NULL);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 5);
    ASSERT_EQ(safe_free_fake.arg0_history[0], &d1);
    ASSERT_EQ(safe_free_fake.arg0_history[1], &list1);
    ASSERT_EQ(safe_free_fake.arg0_history[2], &list2);
    ASSERT_EQ(safe_free_fake.arg0_history[3], &d3);
    ASSERT_EQ(safe_free_fake.arg0_history[4], &list3);
}

void delete_func(void* element_data)
{
    if (element_data)
        safe_free(element_data);
}

TEST_F(list_tests, delete_list_and_data_when_data_not_null_and_delete_function_supplied_deletes_data)
{
    // Arrange
    int d1 = 1;
    int d3 = 3;
    _list_t list3 = {&d3, nullptr};
    _list_t list2 = {nullptr, &list3};
    _list_t list1 = {&d1, &list2};

    // Act
    delete_list_and_data(&list1, delete_func);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 5);
    ASSERT_EQ(safe_free_fake.arg0_history[0], &d1);
    ASSERT_EQ(safe_free_fake.arg0_history[1], &list1);
    ASSERT_EQ(safe_free_fake.arg0_history[2], &list2);
    ASSERT_EQ(safe_free_fake.arg0_history[3], &d3);
    ASSERT_EQ(safe_free_fake.arg0_history[4], &list3);
}

TEST_F(list_tests, delete1_list_deletes_correct_number)
{
    // Arrange
    _list_t list1 = {nullptr, nullptr};

    // Act
    delete1_list(&list1);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(list_tests, list_concat_when_lists_nullptr_returns_nullptr)
{
    // Arrange
    // Act
    auto list = list_concat(nullptr, nullptr);

    //Assert
    ASSERT_EQ(list, nullptr);
}

TEST_F(list_tests, list_concat_when_list1_nullptr_returns_list2)
{
    // Arrange
    _list_t list2{nullptr, nullptr};;

    // Act
    auto list = list_concat(nullptr, &list2);

    //Assert
    ASSERT_EQ(list, &list2);
}

TEST_F(list_tests, list_concat_when_list2_nullptr_returns_list1_next_nullptr)
{
    // Arrange
    _list_t list1{nullptr, nullptr};;

    // Act
    auto list = list_concat(&list1, nullptr);

    //Assert
    ASSERT_EQ(list, &list1);
    ASSERT_EQ(list1.next, nullptr);
}

TEST_F(list_tests, list_concat_when_lists_valid_succeeds)
{
    // Arrange
    _list_t list1_end{nullptr, nullptr};
    _list_t list1{nullptr, &list1_end};

    _list_t list2_end{nullptr, nullptr};
    _list_t list2{nullptr, &list2_end};

    // Act
    auto list = list_concat(&list1, &list2);

    //Assert
    ASSERT_EQ(list, &list1);
    ASSERT_EQ(list1_end.next, &list2);
}

TEST_F(list_tests, list_append_when_r_new_succeeds_list_parameter_nullptr_returns_new_list_sets_data)
{
    // Arrange
    _list_t list_new{nullptr, nullptr};;
    int data = 10;

    safe_malloc_fake.return_val = &list_new;

    // Act
    _list_t *plist = list_append(nullptr, &data);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(plist, &list_new);
    ASSERT_EQ(list_new.data, &data);
}

TEST_F(list_tests, list_append_when_r_new_succeeds_list_parameter_valid_returns_appended_list_sets_data)
{
    // Arrange
    _list_t list_old_end{nullptr, nullptr};
    _list_t list_old {nullptr, &list_old_end};

    _list_t list_new{nullptr, nullptr};
    int data = 10;

    safe_malloc_fake.return_val = &list_new;

    // Act
    _list_t *plist = list_append(&list_old, &data);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(plist, &list_old);
    ASSERT_EQ(list_old.next, &list_old_end);
    ASSERT_EQ(list_old_end.next, &list_new);
    ASSERT_EQ(list_new.data, &data);
}

TEST_F(list_tests, list_prepend_when_when_r_new_succeeds_new_list_is_prepended)
{
    // Arrange
    _list_t list_old{nullptr, nullptr};

    _list_t list_new{nullptr, nullptr};

    int data = 10;

    safe_malloc_fake.return_val = &list_new;

    // Act
    _list_t *plist = list_prepend(&list_old, &data);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(plist, &list_new);
    ASSERT_EQ(list_new.next, &list_old);
    ASSERT_EQ(list_new.data, &data);
}

TEST_F(list_tests, list_nth_when_n_0_returns_list)
{
    // Arrange
    _list_t list_new{nullptr, nullptr};
    int n = 0;

    // Act
    _list_t *plist = list_nth(&list_new, n);

    //Assert
    ASSERT_EQ(plist, &list_new);
}

TEST_F(list_tests, list_nth_when_n_larger_than_list_length_returns_nullptr)
{
    // Arrange
    _list_t list_new{nullptr, nullptr};
    int n = 3;

    // Act
    _list_t *plist = list_nth(&list_new, n);

    //Assert
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, list_nth_when_list_empty_returns_nullptr)
{
    // Arrange
    int n = 3;

    // Act
    _list_t *plist = list_nth(nullptr, n);

    //Assert
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, list_nth_returns_nth)
{
    // Arrange
    int n = 1;
    int data = 10;

    _list_t list_old_end{&data, nullptr};;
    _list_t list_old{nullptr, &list_old_end};;

    // Act
    _list_t *plist = list_nth(&list_old, n);

    //Assert
    ASSERT_EQ(plist, &list_old_end);
}

TEST_F(list_tests, list_remove_when_data_doesnt_exist_not_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;
    int data_not_found = 4;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list);

    // Act
    _list_t *plist = list_remove(test_list, &data_not_found);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 0);
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_when_data_exists_at_first_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove(test_list, &data1);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
    ASSERT_EQ(safe_free_fake.arg0_val, &list_element0);
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_when_data_exists_at_last_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove(test_list, &data3);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
    ASSERT_EQ(safe_free_fake.arg0_val, &list_element2);
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_when_data_exists_at_middle_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove(test_list, &data2);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
    ASSERT_EQ(safe_free_fake.arg0_val, &list_element1);
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_link_when_data_doesnt_exist_not_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;
    _list_t list_not_found{nullptr, nullptr};

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list);

    // Act
    _list_t *plist = list_remove_link(test_list, &list_not_found);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_link_when_data_exists_at_first_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove_link(test_list, &list_element0);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_link_when_data_exists_at_last_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove_link(test_list, &list_element2);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(actual_size, expected_size);
}

TEST_F(list_tests, list_remove_link_when_data_exists_at_middle_element_deleted)
{
    // Arrange
    int data1 = 1;
    int data2 = 2;
    int data3 = 3;

    _list_t *test_list = CreateList3(&data1, &data2, &data3);
    int expected_size = list_size(test_list) - 1;

    // Act
    _list_t *plist = list_remove_link(test_list, &list_element1);
    int actual_size = list_size(plist);

    //Assert
    ASSERT_EQ(actual_size, expected_size);
}


TEST_F(list_tests, list_str_compare_nullptr_paramters_returns_0)
{
    // Arrange
    // Act
    int actual = list_str_compare_func(nullptr, nullptr);

    //Assert
    ASSERT_EQ(actual, 0);
}


TEST_F(list_tests, list_str_compare_a_valid_b_nullptr_returns_less_than_0)
{
    // Arrange
    const char *a_valid = "a_valid";

    // Act
    int actual = list_str_compare_func((void*)a_valid, nullptr);

    //Assert
    ASSERT_EQ(actual, -1);
}

TEST_F(list_tests, list_str_compare_a_nullptr_b_valid_returns_greater_than_0)
{
    // Arrange
    const char *b_valid = "b_valid";

    // Act
    int actual = list_str_compare_func(nullptr, (void*)b_valid);

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(list_tests, list_str_compare_a_valid_b_valid_returns_strcmp_value)
{
    // Arrange
    const char *a_valid = "a_valid";
    const char *b_valid = "b_valid";
    int expected = strcmp(a_valid, b_valid);

    // Act
    int actual = list_str_compare_func((void*)a_valid, (void*)b_valid);

    //Assert
    ASSERT_EQ(actual, expected);
}

TEST_F(list_tests, list_str_compare_a_invalid_data_b_valid_data_returns_less_than_0)
{
    // Arrange
    const char *a_valid = "a_valid";
    int b_invalid_data = 11;

    // Act
    int actual = list_str_compare_func(&b_invalid_data, (void*)a_valid);

    //Assert
    ASSERT_LT(actual, 0);
}

TEST_F(list_tests, list_str_compare_a_valid_b_invalid_data_type_returns_greater_than_0)
{
    // Arrange
    const char *a_valid = "a_valid";
    int b_invalid_data = 11;

    // Act
    int actual = list_str_compare_func((void*)a_valid, &b_invalid_data);

    //Assert
    ASSERT_GT(actual, 0);
}

TEST_F(list_tests, list_sort_list_nullptr_returns_nullptr)
{
    // Arrange
    // Act
    _list_t *plist = list_sort(nullptr, nullptr);

    //Assert
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, list_sort_list_single_entry_returns_list)
{
    // Arrange
    _list_t list_element{nullptr, nullptr};

    // Act
    _list_t *plist = list_sort(&list_element, nullptr);

    //Assert
    ASSERT_EQ(plist, &list_element);
}

TEST_F(list_tests, list_sort_list_compare_function_nullptr_returns_nullptr)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    // Act
    _list_t *plist = list_sort(test_list, nullptr);

    //Assert
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, list_sort_valid_data_sorts_list)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)b_valid, (void*)c_valid, (void*)a_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, c_valid);
}

TEST_F(list_tests, list_sort_first_entry_nullptr_placed_at_end)
{
    // Arrange
    _list_t *test_list = CreateList3(nullptr, (void*)c_valid, (void*)a_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, c_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, nullptr);
}

TEST_F(list_tests, list_sort_middle_entry_nullptr_placed_at_end)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)b_valid, nullptr, (void*)a_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, nullptr);
}

TEST_F(list_tests, list_sort_last_entry_nullptr_placed_at_end)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)b_valid, (void*)c_valid, nullptr);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, c_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, nullptr);
}

TEST_F(list_tests, list_sort_already_sorted_stays_same)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, c_valid);
}

TEST_F(list_tests, list_sort_larger_list_sorted_correctly)
{
    // Arrange
    _list_t *test_list = CreateList6((void*)b_valid, (void*)c_valid, (void*)a_valid, (void*)f_valid, (void*)e_valid, (void*)d_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, c_valid);
    ASSERT_EQ(list_nth(plist, 3)->data, d_valid);
    ASSERT_EQ(list_nth(plist, 4)->data, e_valid);
    ASSERT_EQ(list_nth(plist, 5)->data, f_valid);
}

TEST_F(list_tests, list_sort_duplicates_sorted_correctly)
{
    // Arrange
    _list_t *test_list = CreateList6((void*)b_valid, (void*)c_valid, (void*)a_valid, (void*)c_valid, (void*)f_valid, (void*)d_valid);

    // Act
    _list_t *plist = list_sort(test_list, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(plist, 0)->data, a_valid);
    ASSERT_EQ(list_nth(plist, 1)->data, b_valid);
    ASSERT_EQ(list_nth(plist, 2)->data, c_valid);
    ASSERT_EQ(list_nth(plist, 3)->data, c_valid);
    ASSERT_EQ(list_nth(plist, 4)->data, d_valid);
    ASSERT_EQ(list_nth(plist, 5)->data, f_valid);
}

TEST_F(list_tests, list_last_when_valid_returns_correct_entry)
{
    // Arrange
  _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    // Act
    list_t *plast = list_last(test_list);

    //Assert
    ASSERT_EQ(plast, &list_element2);
}

TEST_F(list_tests, list_last_when_single_entry_returns_correct_entry)
{
    // Arrange
    list_t single_element;
    single_element.next = nullptr;

    // Act
    list_t *plast = list_last(&single_element);

    //Assert
    ASSERT_EQ(plast, &single_element);
}

TEST_F(list_tests, list_last_when_nullptr_list_returns_nullptr)
{
    // Arrange
    // Act
    list_t *plast = list_last(nullptr);

    //Assert
    ASSERT_EQ(plast, nullptr);
}

TEST_F(list_tests, list_size_when_valid_returns_correct_value)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    // Act
    int actual = list_size(test_list);

    //Assert
    ASSERT_EQ(actual, 3);
}

TEST_F(list_tests, list_size_when_single_entry_returns_correct_value)
{
    // Arrange
    list_t single_element;
    single_element.next = nullptr;

    // Act
    int actual = list_size(&single_element);

    //Assert
    ASSERT_EQ(actual, 1);
}

TEST_F(list_tests, list_size_when_nullptr_list_returns_nullptr)
{
    // Arrange
    // Act
   int actual = list_size(nullptr);

    //Assert
    ASSERT_EQ(actual, 0);
}

TEST_F(list_tests, list_insert_at_front_inserts_correctly)
{
    // Arrange
    _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    safe_malloc_fake.return_val = &list_element3;

    // Act
    _list_t *actual = list_insert_at(test_list, 0, (void*)d_valid);

    //Assert
    ASSERT_EQ(actual->data, d_valid);
    ASSERT_EQ(actual->next, test_list);
}

TEST_F(list_tests, list_insert_at_middle_inserts_correctly)
{
    // Arrange
    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_at(test_list, 2, (void*)g_valid);

    //Assert
    ASSERT_EQ(list_nth(actual, 1)->data, b_valid);
    ASSERT_EQ(list_nth(actual, 2)->data, g_valid);
    ASSERT_EQ(list_nth(actual, 3)->data, c_valid);
}

TEST_F(list_tests, list_insert_at_end_inserts_correctly)
{
    // Arrange
    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_at(test_list, 6, (void*)g_valid);

    //Assert
    ASSERT_EQ(list_nth(actual, 5)->data, f_valid);
    ASSERT_EQ(list_nth(actual, 6)->data, g_valid);
}

TEST_F(list_tests, list_insert_past_end_inserts_at_end)
{
    // Arrange
    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_at(test_list, 8, (void*)g_valid);

    //Assert
    ASSERT_EQ(list_nth(actual, 5)->data, f_valid);
    ASSERT_EQ(list_nth(actual, 6)->data, g_valid);
}


/////

TEST_F(list_tests, list_insert_ordered_nullptr_list_returns_new_entry)
{
    // Arrange
    safe_malloc_fake.return_val = &list_element3;

    // Act
    _list_t *actual = list_insert_ordered(nullptr, (void*)a_valid, list_str_compare_func);

    //Assert
    ASSERT_EQ(actual, &list_element3);
}

TEST_F(list_tests, list_insert_ordered_front_inserts_correctly)
{
    // Arrange
    const char *a_avalid = "a_avalid";
    _list_t *test_list = CreateList3((void*)a_valid, (void*)b_valid, (void*)c_valid);

    safe_malloc_fake.return_val = &list_element3;

    // Act
    _list_t *actual = list_insert_ordered(test_list, (void*)a_avalid, list_str_compare_func);

    //Assert
    ASSERT_EQ(actual->data, a_avalid);
    ASSERT_EQ(actual->next, test_list);
}

TEST_F(list_tests, list_insert_ordered_at_middle_inserts_correctly)
{
    // Arrange
    const char *c_cvalid = "c_cvalid";

    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_ordered(test_list, (void*)c_cvalid, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(actual, 2)->data, c_cvalid);
    ASSERT_EQ(list_nth(actual, 3)->data, c_valid);
    ASSERT_EQ(list_nth(actual, 4)->data, d_valid);
}

TEST_F(list_tests, list_insert_ordered_at_end_inserts_correctly)
{
    // Arrange
    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_ordered(test_list, (void*)g_valid, list_str_compare_func);

    //Assert
    ASSERT_EQ(list_nth(actual, 5)->data, f_valid);
    ASSERT_EQ(list_nth(actual, 6)->data, g_valid);
}

TEST_F(list_tests, list_insert_ordered_compare_func_nullptr_returns_nullptr)
{
    // Arrange
    list_t new_element;
    new_element.next = nullptr;

    _list_t *test_list = CreateList6((void*)a_valid, (void*)b_valid, (void*)c_valid, (void*)d_valid, (void*)e_valid, (void*)f_valid);

    safe_malloc_fake.return_val = &new_element;

    // Act
    _list_t *actual = list_insert_ordered(test_list, (void*)g_valid, nullptr);

    //Assert
    ASSERT_EQ(actual, nullptr);
}
