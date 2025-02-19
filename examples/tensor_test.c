#include<binsparse/tensor.h>
#include<binsparse/read_tensor.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: ./tensor_test [file_name.h5]\n");
    return 1;
  }
  char* file_name = argv[1];
  bsp_tensor_t tensor = bsp_read_tensor(file_name, NULL);
  printf("rank: %d\n", tensor.rank);
  printf("dims:");
  for (int i = 0; i < tensor.rank; i++) {
    printf("%ld, ", tensor.dims[i]);
  }
  printf("\n");
  bsp_destroy_tensor_t(tensor);
  return 0;
}
