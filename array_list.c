/*
 * Copyright 2010-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws/common/array_list.h>
#include <aws/common/private/array_list.h>
#include "proof_helpers/make_common_data_structures.h"
#include "aws_seahorn.h"

extern void ndset(char*, int);
extern size_t nd_sizet();
extern char* nd_ptr();

bool aws_array_list_is_bounded(
    const struct aws_array_list *const list,
    const size_t max_initial_item_allocation,
    const size_t max_item_size) {
    bool item_size_is_bounded = list->item_size <= max_item_size;
    bool length_is_bounded = list->length <= max_initial_item_allocation;
    return item_size_is_bounded && length_is_bounded;
}

void ensure_array_list_has_allocated_data_member(struct aws_array_list *const list) {
    list->current_size = nd_sizet();
    list->length = nd_sizet();
    list->item_size = nd_sizet();
    /// XXX Cannot do this for now
    //list->data = bounded_malloc(list->current_size);
    list->data = bounded_malloc(MAX_INITIAL_ITEM_ALLOCATION * MAX_ITEM_SIZE);

    /// XXX Need to take care of fail allocator
    if (list->current_size == 0 && list->length == 0) {
        assume(list->data == NULL);
        list->alloc = NULL;//can_fail_allocator();
    } else {
        list->alloc = NULL;//nondet_bool() ? NULL : can_fail_allocator();
    }
}

int aws_array_list_copy(const struct aws_array_list *AWS_RESTRICT from, struct aws_array_list *AWS_RESTRICT to) {
    AWS_FATAL_PRECONDITION(from->item_size == to->item_size);
    AWS_FATAL_PRECONDITION(from->data);
    AWS_PRECONDITION(aws_array_list_is_valid(from));
    AWS_PRECONDITION(aws_array_list_is_valid(to));

    size_t copy_size = 0;
    if (aws_mul_size_checked(from->length, from->item_size, &copy_size)) {
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_ERR;
    }

    if (to->current_size >= copy_size) {
        if (copy_size > 0) {
            /// XXX Not needed for now
            //memcpy(to->data, from->data, copy_size);
        }
        to->length = from->length;
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_SUCCESS;
    }

    /* if to is in dynamic mode, we can just reallocate it and copy */
    if (to->alloc != NULL) {
        void *tmp = aws_mem_acquire(to->alloc, copy_size);

        if (!tmp) {
            AWS_POSTCONDITION(aws_array_list_is_valid(from));
            AWS_POSTCONDITION(aws_array_list_is_valid(to));
            return AWS_OP_ERR;
        }

        memcpy(tmp, from->data, copy_size);
        if (to->data) {
            aws_mem_release(to->alloc, to->data);
        }

        to->data = tmp;
        to->length = from->length;
        to->current_size = copy_size;
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_SUCCESS;
    }

    return aws_raise_error(AWS_ERROR_DEST_COPY_TOO_SMALL);
}

int main() {
    /* data structure */
    struct aws_array_list from;
    struct aws_array_list to;

#ifdef INIT
    from.current_size = nd_sizet();
    from.length = nd_sizet();
    assume (from.current_size > 0 && from.length > 0);
    from.item_size = nd_sizet();
    from.data = bounded_malloc(MAX_INITIAL_ITEM_ALLOCATION * MAX_ITEM_SIZE);
    from.alloc = NULL;

    to.current_size = nd_sizet();
    to.length = nd_sizet();
    assume (to.current_size > 0 && to.length > 0);
    to.item_size = nd_sizet();
    to.data = bounded_malloc(MAX_INITIAL_ITEM_ALLOCATION * MAX_ITEM_SIZE);
    to.alloc = NULL;
#endif

    /* assumptions */
    assume(aws_array_list_is_bounded(&from, MAX_INITIAL_ITEM_ALLOCATION, MAX_ITEM_SIZE));
    #ifndef INIT
    ensure_array_list_has_allocated_data_member(&from);
    #endif
    assume(aws_array_list_is_valid(&from));

    assume(aws_array_list_is_bounded(&to, MAX_INITIAL_ITEM_ALLOCATION, MAX_ITEM_SIZE));
    #ifndef INIT
    ensure_array_list_has_allocated_data_member(&to);
    #endif
    assume(aws_array_list_is_valid(&to));

    assume(from.item_size == to.item_size);
    assume(from.data != NULL);

    /* perform operation under verification */
    if (!aws_array_list_copy(&from, &to)) {
        /* In the case aws_array_list_copy is successful, both lists have the same length */
        sassert(to.length == from.length);
        sassert(to.current_size >= (from.length * from.item_size));
    }

    /* assertions */
    sassert(aws_array_list_is_valid(&from));
    sassert(aws_array_list_is_valid(&to));
    sassert(from.item_size == to.item_size);

    return 0;
}