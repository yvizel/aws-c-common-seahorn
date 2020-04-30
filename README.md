# aws-c-common-seahorn

# Use with and without -DINIT for the two options
sea bpf array_list.c -I/Users/yakir/workspace/aws-c-common-seahorn/:/Users/yakir/workspace/aws-c-common/include/:/Users/yakir/workspace/aws-c-common/.cbmc-batch/include/ --bmc=mono --dsa=sea-cs --horn-bv2=true --log=opsem   --oll=out.ll --inline -O0 --log=cex --cex=/tmp/h.ll
