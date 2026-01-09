/**
 * @author Sean Hobeck
 * @date 2026-01-07
 */
#include <stdio.h>

int main(int argc, char** argv) {

    /**
     *
     * client write thread rough:
     *
     * ask for data to find the head commit id of the server, compute the commit difference
     * send off a list of each of the commits as well as their changes afterwards.
     *
     *
     * server read thread rough:
     *
     * sockfd
     * FILE* sock_stream = fdopen(sockfd, "r");
     * if (sock_stream == 0x0) {
     *      fprintf(stderr, "fdopen failed; socket stream could not be opened for writing.\n");
     *      exit(EXIT_FAILURE); // exit on failure
     * }
     *
     * some loop with a protocol enum ...
     * branch_t* read_branch = read_branch_from_stream(sock_stream);
     * fflush(sock_stream);
     *
     * read_branch->path = strdup("some literal configured absolute path");
     * write_branch(read_branch);
     * free(read_branch);
     *
     *
     * say you have a branch with x number of commits, and each of those commits have y number of
     * changes/ diffs. how do you know which change is appointed to which commit? we simply read
     * them without order, that being we simply take in the information before handling it later on.
     *
     * consider the situation in which we are given one commit with ten changes. if we read 3
     * changes, then a commit, then 7 more changes we simply throw all of the changes and commits
     * into what we call a pool. this pool then takes the higher order data (changes/ diffs ->
     * commits -> branches) and starts placing the information where it needs to, finalizing the
     * data for writing.
     *
     * what happens if we are left over with some information in any one of the pools? this is a
     * security issue and we need to notify the pusher that we cannot accept the changes that
     * have been given. EPOOLNONNULL.
     *
     *
     * client read-back thread:
     *
     * did EPOOLNONNULL happen? if it did we simply need to notify the user that there is
     * something wrong with the repository, ie.
     *
     * lit commit -m "made some changes, ready for push!"
     * lit push --upstream 127.0.0.1:8923
     * push failed; EPOOLNONNULL. extra data sent without relevance, see 'lit restore'.
     */

    return 0;
}