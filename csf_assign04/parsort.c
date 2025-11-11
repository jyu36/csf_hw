#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

int compare( const void *left, const void *right );
void swap( int64_t *arr, unsigned long i, unsigned long j );
unsigned long partition( int64_t *arr, unsigned long start, unsigned long end );
int quicksort( int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold );

// TODO: declare additional helper functions if needed

// Structure to keep track of child process information
typedef struct {
    pid_t pid;        // process ID of the child (if created)
    int waited;       // whether waitpid() has been called
    int success;      // whether the child completed successfully
    int valid;        // whether a child was successfully created
} Child;

Child quicksort_subproc(int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold);
void quicksort_wait(Child *child);
int quicksort_check_success(Child *child);
static int quicksort_parallel(int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold);

int main( int argc, char **argv ) {
  unsigned long par_threshold;
  if ( argc != 3 || sscanf( argv[2], "%lu", &par_threshold ) != 1 ) {
    fprintf( stderr, "Usage: parsort <file> <par threshold>\n" );
    exit( 1 );
  }

  int fd;

  fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Error: can't open file '%s'\n", argv[1]);
    exit(1);
  }

  // determine file size and number of elements
  unsigned long file_size, num_elements;
  struct stat statbuf;
  int rc = fstat(fd, &statbuf);
  if (rc != 0) {
    fprintf(stderr, "Error: fstat failed\n");
    close(fd);
    exit(1);
  }
  file_size = statbuf.st_size;
  num_elements = file_size / sizeof(int64_t);

  // mmap the file data
  int64_t *arr;
  arr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd); // file descriptor can be closed after mmap
  if (arr == MAP_FAILED) {
    fprintf(stderr, "Error: mmap failed\n");
    exit(1);
  }

  // Sort the data!
  int success;
  success = quicksort( arr, 0, num_elements, par_threshold );
  if ( !success ) {
    fprintf( stderr, "Error: sorting failed\n" );
    exit( 1 );
  }

  // Unmap the file data
  munmap(arr, file_size);

  return 0;
}

// Compare elements.
// This function can be used as a comparator for a call to qsort.
//
// Parameters:
//   left - pointer to left element
//   right - pointer to right element
//
// Return:
//   negative if *left < *right,
//   positive if *left > *right,
//   0 if *left == *right
int compare( const void *left, const void *right ) {
  int64_t left_elt = *(const int64_t *)left, right_elt = *(const int64_t *)right;
  if ( left_elt < right_elt )
    return -1;
  else if ( left_elt > right_elt )
    return 1;
  else
    return 0;
}

// Swap array elements.
//
// Parameters:
//   arr - pointer to first element of array
//   i - index of element to swap
//   j - index of other element to swap
void swap( int64_t *arr, unsigned long i, unsigned long j ) {
  int64_t tmp = arr[i];
  arr[i] = arr[j];
  arr[j] = tmp;
}

// Partition a region of given array from start (inclusive)
// to end (exclusive).
//
// Parameters:
//   arr - pointer to first element of array
//   start - inclusive lower bound index
//   end - exclusive upper bound index
//
// Return:
//   index of the pivot element, which is globally in the correct place;
//   all elements to the left of the pivot will have values less than
//   the pivot element, and all elements to the right of the pivot will
//   have values greater than or equal to the pivot
unsigned long partition( int64_t *arr, unsigned long start, unsigned long end ) {
  assert( end > start );

  // choose the middle element as the pivot
  unsigned long len = end - start;
  assert( len >= 2 );
  unsigned long pivot_index = start + (len / 2);
  int64_t pivot_val = arr[pivot_index];

  // stash the pivot at the end of the sequence
  swap( arr, pivot_index, end - 1 );

  // partition all of the elements based on how they compare
  // to the pivot element: elements less than the pivot element
  // should be in the left partition, elements greater than or
  // equal to the pivot should go in the right partition
  unsigned long left_index = start,
                right_index = start + ( len - 2 );

  while ( left_index <= right_index ) {
    // extend the left partition?
    if ( arr[left_index] < pivot_val ) {
      ++left_index;
      continue;
    }

    // extend the right partition?
    if ( arr[right_index] >= pivot_val ) {
      --right_index;
      continue;
    }

    // left_index refers to an element that should be in the right
    // partition, and right_index refers to an element that should
    // be in the left partition, so swap them
    swap( arr, left_index, right_index );
  }

  // at this point, left_index points to the first element
  // in the right partition, so place the pivot element there
  // and return the left index, since that's where the pivot
  // element is now
  swap( arr, left_index, end - 1 );
  return left_index;
}

// Sort specified region of array.
// Note that the only reason that sorting should fail is
// if a child process can't be created or if there is any
// other system call failure.
//
// Parameters:
//   arr - pointer to first element of array
//   start - inclusive lower bound index
//   end - exclusive upper bound index
//   par_threshold - if the number of elements in the region is less
//                   than or equal to this threshold, sort sequentially,
//                   otherwise sort in parallel using child processes
//
// Return:
//   1 if the sort was successful, 0 otherwise
int quicksort(int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold) {
    assert(end >= start);
    unsigned long len = end - start;

    // Base case: fewer than 2 elements
    if (len < 2)
        return 1;

    // Sequential sort if below threshold
    if (len <= par_threshold) {
        qsort(arr + start, len, sizeof(int64_t), compare);
        return 1;
    } else {
        //recursive case: parallel quicksort
        return quicksort_parallel(arr, start, end, par_threshold);
    }
}

// TODO: define additional helper functions if needed

// Function to create a child process to perform quicksort on a subarray
Child quicksort_subproc(int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold) {
    Child child = { .pid = -1, .waited = 0, .success = 0, .valid = 0 };
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return child; // invalid child
    }
    if (pid == 0) {
        // Child process — perform recursive sort
        int success = quicksort(arr, start, end, par_threshold);
        exit(success ? 0 : 1);
    }
    // Parent process — store child info
    child.pid = pid;
    child.valid = 1;
    return child;
}

// Waits for a child process (created by quicksort_subproc) to complete,
// verifies its exit status, and updates the child's success and waited flags.
void quicksort_wait(Child *child) {
    if (!child->valid || child->waited)
        return;
    int wstatus;
    if (waitpid(child->pid, &wstatus, 0) < 0) {
        perror("waitpid failed");
        child->success = 0;
    } else if (!WIFEXITED(wstatus)) {
        fprintf(stderr, "Child %d did not exit normally\n", child->pid);
        child->success = 0;
    } else if (WEXITSTATUS(wstatus) != 0) {
        fprintf(stderr, "Child %d exited with code %d\n", child->pid, WEXITSTATUS(wstatus));
        child->success = 0;
    } else {
        child->success = 1;
    }
    child->waited = 1;
}

//Create Child Process to do recursive sortingif size > threshold
static int quicksort_parallel(int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold) {
    unsigned long mid = partition(arr, start, end);

    Child left = quicksort_subproc(arr, start, mid, par_threshold);
    Child right = quicksort_subproc(arr, mid + 1, end, par_threshold);
    
    // Check if both children were created successfully
    if (!left.valid || !right.valid) {
        // Fork failed - wait for any successfully created children
        if (left.valid) quicksort_wait(&left);
        if (right.valid) quicksort_wait(&right);
        return 0;
    }
    
    // Wait for both child processes to finish and verify success
    quicksort_wait(&left);
    quicksort_wait(&right);
    
    // Return 1 only if both left and right sides sorted successfully
    return left.success && right.success;
}
