#include <string>
#include "gtest/gtest.h"

extern "C" {
#include "fff.h"
}

DEFINE_FFF_GLOBALS;

#include "r.h"
#include "list.h"

FAKE_VALUE_FUNC(void *, safe_malloc, size_t, int);
FAKE_VOID_FUNC(safe_free, void *);
FAKE_VOID_FUNC_VARARG(r_err, const char*, ...);

class list_tests : public ::testing::Test
{
protected:
	list_tests()
    {
	}

	virtual ~list_tests()
    {
	}

	virtual void SetUp() 
    {
        RESET_FAKE(safe_malloc);
        RESET_FAKE(safe_free);
        RESET_FAKE(r_err);
	}

	virtual void TearDown() 
    {
	}

};

TEST_F(list_tests, new_list_when_r_new_fails_logs_returns_NULL)
{
    // Arrange
    safe_malloc_fake.return_val = NULL;

    // Act
    _list_t *plist = new_list(NULL);

    //AssertNULL
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 1);
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, new_list_when_r_new_succeeds_returns_valid_pointer)
{
    // Arrange
    _list_t list;

    safe_malloc_fake.return_val = &list;

    // Act
    _list_t *plist = new_list(NULL);

    //AssertNULL
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(r_err_fake.call_count, 0);
    ASSERT_EQ(plist, &list);
}

TEST_F(list_tests, new_list_when_r_new_succeeds_sets_data_next_NULL)
{
    // Arrange
    _list_t list;
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

TEST_F(list_tests, delete_list_when_null_pointer_passed_does_not_delete)
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
    _list_t list = {.data = NULL, .next = NULL};

    // Act
    delete_list(&list);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(list_tests, delete_list_multiple_list_entry_deletes_correct_number)
{
    // Arrange
    _list_t list3 = {.data = NULL, .next = NULL};
    _list_t list2 = {.data = NULL, .next = &list3};
    _list_t list1 = {.data = NULL, .next = &list2};

    // Act
    delete_list(&list1);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 3);
}

TEST_F(list_tests, delete1_list_deletes_correct_number)
{
    // Arrange

    _list_t list1 = {.data = NULL, .next = NULL};

    // Act
    delete1_list(&list1);

    //Assert
    ASSERT_EQ(safe_free_fake.call_count, 1);
}

TEST_F(list_tests, list_concat_when_lists_null_returns_null)
{
    // Arrange
    // Act
    auto list = list_concat(nullptr, nullptr);

    //Assert
    ASSERT_EQ(list, nullptr);
}

TEST_F(list_tests, list_concat_when_list1_null_returns_list2)
{
    // Arrange
    _list_t list2;

    // Act
    auto list = list_concat(nullptr, &list2);

    //Assert
    ASSERT_EQ(list, &list2);
}

TEST_F(list_tests, list_concat_when_list2_null_returns_list1_next_NULL)
{
    // Arrange
    _list_t list1;
    list1.next = nullptr;

    // Act
    auto list = list_concat(&list1, nullptr);

    //Assert
    ASSERT_EQ(list, &list1);
    ASSERT_EQ(list1.next, nullptr);
}

TEST_F(list_tests, list_concat_when_lists_valid_succeeds)
{
    // Arrange
    _list_t list1;
    _list_t list1_end;
    list1.next = &list1_end;
    list1_end.next = nullptr;

    _list_t list2;
    _list_t list2_end;
    list2.next = &list2_end;
    list2_end.next = nullptr;

    // Act
    auto list = list_concat(&list1, &list2);

    //Assert
    ASSERT_EQ(list, &list1);
    ASSERT_EQ(list1_end.next, &list2);
}

TEST_F(list_tests, list_append_when_r_new_fails_returns_NULL)
{
    // Arrange

    safe_malloc_fake.return_val = NULL;

    // Act
    _list_t *plist = list_append(nullptr, nullptr);

    //Assert
    ASSERT_EQ(safe_malloc_fake.call_count, 1);
    ASSERT_EQ(plist, nullptr);
}

TEST_F(list_tests, list_append_when_r_new_succeeds_list_parameter_null_returns_new_list_sets_data)
{
    // Arrange
    _list_t list_new;
    list_new.next = nullptr;
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
    _list_t list_old;
    _list_t list_old_end;
    list_old.next = &list_old_end;
    list_old_end.next = nullptr;

    _list_t list_new;
    list_new.next = nullptr;
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