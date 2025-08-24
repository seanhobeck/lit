/**
 * @author Sean Hobeck
 * @date 2025-08-23
 *
 * @file rebase.c
 *    the rebase module, responsible for rebasing, and checking collisions
 *    between the rebasing of branches.
 */
#include "rebase.h"

/*! @uses apply_forward_commit */
#include "ops.h"

/*! @uses bool, true, false */
#include <stdbool.h>

/**
 * @brief are the two commits conflicting in their changes at all?
 *
 * @param first the first commit to compare.
 * @param second the second commit to compare.
 * @return true if there are any commit conflicts (printed to stderr).
 */
bool
is_conflicting_commits(const commit_t* first, const commit_t* second) {
    // assert on the commits.
    assert(first != 0x0);
    assert(second != 0x0);
    bool is_conflicting = false;

    // iterate through each change in the first commit
    for (size_t i = 0; i < first->count; i++) {
        diff_t* first_change = first->changes[i];

        // iterate through each change in the second commit.
        for (size_t j = 0; j < second->count; j++) {
            diff_t* second_change = second->changes[j];

            // if the filenames match at all, there could be a conflict.
            if (!strcmp(first_change->new_path, second_change->new_path)) {
                is_conflicting = true;
                fprintf(stderr, "found conflicting changes in %s/@%u vs %s/@%u\n", \
                    strsha1(first->hash), first_change->crc, strsha1(second->hash), second_change->crc);
            }
        }
    }
    return is_conflicting;
}

/// @note minimum function macro.
#define min(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief if the branch <current> can rebase commits ontop of <source> (without modifications).
 *
 * @param repository the repository read from the cwd.
 * @param destination_branch_name the destination branch name.
 * @param source_branch_name the source branch name.
 * @return if rebase is possible on branch <source>.
 */
bool
is_rebase_possible(const repository_t* repository, const char* destination_branch_name, \
    const char* source_branch_name) {
    // assert on the repository, destination and then branch name.
    assert(repository != 0x0);
    assert(destination_branch_name != 0x0);
    assert(destination_branch_name != 0x0);

    // find the current and active branches.
    branch_t* destination = get_branch_repository(repository, destination_branch_name),\
    * source = get_branch_repository(repository, source_branch_name);

    // find the common ancestor.
    commit_t* ancestor = find_common_ancestor(destination, source);
    if (!ancestor) {
        fprintf(stderr, "no common ancestor found between \'%s\' and \'%s\'\n", \
            destination->name, source->name);
        return false;
    };
    size_t ancestor_idx = find_index_commit(destination, ancestor);
    if (ancestor_idx == (size_t) -1) {
        fprintf(stderr, "ancestor commit \'%s\' not found in \'%s\'\n", \
            strsha1(ancestor->hash), destination->name);
        return false;
    }

    // check each change in commits after the ancestor on <source>
    //  if there is a change that could rise a conflict in history
    //  report the conflict found, continue, and then eventually
    //  return false.
    bool conflict_found = false;
    for (size_t i = ancestor_idx + 1; i < source->idx && i < min(source->count, destination->count); i++) {
        // get the commit after the ancestor on the source branch.
        commit_t* source_commit = source->commits[i], * destination_commit = destination->commits[i];

        // check if the commits have any conflicts.
        if (is_conflicting_commits(source_commit, destination_commit))
            conflict_found = true;
    }
    return !conflict_found;
};

/**
 * @brief rebase the current/active branch onto source branch.
 *
 * @param repository the repository read from the cwd.
 * @param destination_branch_name the destination branch to rebase onto.
 * @param source_branch_name the source branch to rebase from.
 * @return rebase result status enum.
 */
e_rebase_result_t
rebase_branch(const repository_t* repository, const char* destination_branch_name, \
    const char* source_branch_name) {
    // assert on the repository, destination and then branch name.
    assert(repository != 0x0);
    assert(destination_branch_name != 0x0);
    assert(destination_branch_name != 0x0);

    // check if the rebase is even possible first.
    if (!is_rebase_possible(repository, destination_branch_name, source_branch_name)) {
        fprintf(stderr, "rebase is not possible on branch \'%s\', "
                        "conflicts or errors found (please fix), see above.\n", source_branch_name);
        return E_REBASE_RESULT_CONFLICT;
    }

    // find the source and destination branches.
    branch_t* destination = get_branch_repository(repository, destination_branch_name),\
    * source = get_branch_repository(repository, source_branch_name);

    // find the common ancestor
    commit_t* ancestor = find_common_ancestor(destination, source);
    size_t source_ancestor_idx = find_index_commit(source, ancestor);
    size_t rebase_count = source->idx - source_ancestor_idx;

    // calculate the commit count to be added, and then add them onto the dest.
    for (size_t i = source_ancestor_idx + 1; i < source->count; i++) {
        commit_t* commit = source->commits[i];
        add_commit_branch(commit, destination);
    }

    // checkout the most recent commits just added if it is the active branch.
    if (!strcmp(destination->name, repository->branches[repository->idx]->name))
        checkout(destination, destination->commits[destination->idx + rebase_count]);
    else destination->idx += rebase_count;

    // write and log.
    write_branch(destination);
    write_repository(repository);
    printf("successfully rebased \'%s\' onto \'%s\' with %lu commit(s).\n", \
        source_branch_name, destination->name, rebase_count);
    return E_REBASE_RESULT_SUCCESS;
};