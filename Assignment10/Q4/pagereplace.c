#include "kernel/types.h"
#include "user/user.h"

#define REF_COUNT 20
#define MAX_FRAMES 5

enum policy {
  FIFO,
  LRU,
  OPTIMAL
};

static char *policy_name[] = {"FIFO", "LRU", "Optimal"};

static int
find_empty(int frames[], int count)
{
  for (int i = 0; i < count; i++) {
    if (frames[i] == -1)
      return i;
  }
  return -1;
}

static int
find_page(int frames[], int count, int page)
{
  for (int i = 0; i < count; i++) {
    if (frames[i] == page)
      return i;
  }
  return -1;
}

static int
choose_optimal_victim(int frames[], int count, int refs[], int length, int at)
{
  int victim = 0;
  int farthest = -1;

  for (int frame = 0; frame < count; frame++) {
    int next;
    for (next = at + 1; next < length; next++) {
      if (refs[next] == frames[frame])
        break;
    }
    if (next == length)
      return frame;
    if (next > farthest) {
      farthest = next;
      victim = frame;
    }
  }
  return victim;
}

static void
simulate(enum policy policy, int frame_count, int refs[], int ref_count,
         int verbose, int *faults_out, int *hits_out)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int next_fifo = 0;
  int faults = 0;
  int hits = 0;

  for (int i = 0; i < frame_count; i++) {
    frames[i] = -1;
    last_used[i] = -1;
  }
  if (verbose)
    printf("\n%s with %d frames:\n", policy_name[policy], frame_count);

  for (int i = 0; i < ref_count; i++) {
    int page = refs[i];
    int slot = find_page(frames, frame_count, page);
    int hit = slot >= 0;

    if (hit) {
      hits++;
      last_used[slot] = i;
    } else {
      int empty = find_empty(frames, frame_count);
      faults++;
      if (policy == FIFO) {
        slot = next_fifo;
        next_fifo = (next_fifo + 1) % frame_count;
      } else if (empty >= 0) {
        slot = empty;
      } else if (policy == LRU) {
        slot = 0;
        for (int j = 1; j < frame_count; j++) {
          if (last_used[j] < last_used[slot])
            slot = j;
        }
      } else {
        slot = choose_optimal_victim(frames, frame_count, refs, ref_count, i);
      }
      frames[slot] = page;
      last_used[slot] = i;
    }

    if (verbose) {
      printf("Ref %d: [", page);
      for (int j = 0; j < frame_count; j++) {
        if (j > 0)
          printf(" ");
        if (frames[j] < 0)
          printf("-");
        else
          printf("%d", frames[j]);
      }
      printf("] %s\n", hit ? "HIT" : "FAULT");
    }
  }

  *faults_out = faults;
  *hits_out = hits;
}

int
main(void)
{
  int refs[REF_COUNT] = {
    7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1
  };
  int faults, hits;

  for (int policy = FIFO; policy <= OPTIMAL; policy++) {
    simulate((enum policy)policy, 3, refs, REF_COUNT, 1, &faults, &hits);
    printf("%s with 3 frames: %d faults, %d hits\n",
           policy_name[policy], faults, hits);
  }

  printf("\nFrames | Algorithm | Faults | Hits | Hit ratio | Fault ratio\n");
  for (int frame_count = 3; frame_count <= MAX_FRAMES; frame_count++) {
    for (int policy = FIFO; policy <= OPTIMAL; policy++) {
      simulate((enum policy)policy, frame_count, refs, REF_COUNT, 0,
               &faults, &hits);
      printf("%6d | %9s | %6d | %4d | %8d%% | %11d%%\n",
             frame_count, policy_name[policy], faults, hits,
             hits * 100 / REF_COUNT, faults * 100 / REF_COUNT);
    }
  }
  exit(0);
}
