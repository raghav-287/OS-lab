#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_REF 100

void
simulate_fifo(int num_frames, int refs[], int ref_len, int *faults, int *hits)
{
  int frames[10];
  int head = 0;
  *faults = 0;
  *hits = 0;

  for(int i = 0; i < num_frames; i++) frames[i] = -1;

  printf("\n--- FIFO Replacement (%d frames) ---\n", num_frames);
  for(int i = 0; i < ref_len; i++){
    int page = refs[i];
    int hit = 0;

    for(int j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        break;
      }
    }

    if(hit){
      (*hits)++;
      printf("Ref %d: Frames [", page);
    } else {
      (*faults)++;
      frames[head] = page;
      head = (head + 1) % num_frames;
      printf("Ref %d: Frames [", page);
    }

    for(int j = 0; j < num_frames; j++){
      if(frames[j] != -1) printf("%d ", frames[j]);
      else printf("- ");
    }
    printf("] %s\n", hit ? "HIT" : "FAULT");
  }
}

void
simulate_lru(int num_frames, int refs[], int ref_len, int *faults, int *hits)
{
  int frames[10];
  int last_used[10];
  *faults = 0;
  *hits = 0;

  for(int i = 0; i < num_frames; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  printf("\n--- LRU Replacement (%d frames) ---\n", num_frames);
  for(int i = 0; i < ref_len; i++){
    int page = refs[i];
    int hit = 0;

    for(int j = 0; j < num_frames; j++){
      if(frames[j] == page){
        hit = 1;
        last_used[j] = i;
        break;
      }
    }

    if(hit){
      (*hits)++;
      printf("Ref %d: Frames [", page);
    } else {
      (*faults)++;
      // Find empty slot or least recently used frame
      int lru_idx = 0;
      int min_time = 1e9;
      for(int j = 0; j < num_frames; j++){
        if(frames[j] == -1){
          lru_idx = j;
          break;
        }
        if(last_used[j] < min_time){
          min_time = last_used[j];
          lru_idx = j;
        }
      }
      frames[lru_idx] = page;
      last_used[lru_idx] = i;
      printf("Ref %d: Frames [", page);
    }

    for(int j = 0; j < num_frames; j++){
      if(frames[j] != -1) printf("%d ", frames[j]);
      else printf("- ");
    }
    printf("] %s\n", hit ? "HIT" : "FAULT");
  }
}

int
main(void)
{
  int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int ref_len = 20;

  int fifo_faults, fifo_hits;
  int lru_faults, lru_hits;

  // Question 3b test with 3 frames
  simulate_fifo(3, refs, ref_len, &fifo_faults, &fifo_hits);
  simulate_lru(3, refs, ref_len, &lru_faults, &lru_hits);

  // Question 3c summary table
  printf("\n=======================================================\n");
  printf("  Frames | Algo | Faults | Hits | Hit Ratio (%%)       \n");
  printf("=======================================================\n");

  for(int f = 3; f <= 5; f++){
    simulate_fifo(f, refs, ref_len, &fifo_faults, &fifo_hits);
    simulate_lru(f, refs, ref_len, &lru_faults, &lru_hits);

    int fifo_ratio = (fifo_hits * 100) / ref_len;
    int lru_ratio = (lru_hits * 100) / ref_len;

    printf("    %d    | FIFO |   %d   |  %d   | %d%%\n", f, fifo_faults, fifo_hits, fifo_ratio);
    printf("    %d    | LRU  |   %d   |  %d   | %d%%\n", f, lru_faults, lru_hits, lru_ratio);
  }
  printf("=======================================================\n");

  exit(0);
}