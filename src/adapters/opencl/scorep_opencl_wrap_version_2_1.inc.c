cl_int
SCOREP_LIBWRAP_WRAPPER( clGetKernelSubGroupInfo )( cl_kernel                kernel,
                                                   cl_device_id             device,
                                                   cl_kernel_sub_group_info paramName,
                                                   size_t                   inputValueSize,
                                                   const void*              inputValue,
                                                   size_t                   paramValueSize,
                                                   void*                    paramValue,
                                                   size_t*                  paramValueSizeRet )
{
    SCOREP_OPENCL_FUNC_ENTER( clGetKernelSubGroupInfo );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_int ret = SCOREP_LIBWRAP_ORIGINAL( clGetKernelSubGroupInfo )(
        kernel, device, paramName, inputValueSize, inputValue, paramValueSize, paramValue, paramValueSizeRet );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clGetKernelSubGroupInfo );

    return ret;
}



cl_program
SCOREP_LIBWRAP_WRAPPER( clCreateProgramWithIL )( cl_context  context,
                                                 const void* il,
                                                 size_t      length,
                                                 cl_int*     errCodeRet )
{
    SCOREP_OPENCL_FUNC_ENTER( clCreateProgramWithIL );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_program ret = SCOREP_LIBWRAP_ORIGINAL( clCreateProgramWithIL )(
        context, il, length, errCodeRet );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clCreateProgramWithIL );

    return ret;
}



cl_int
SCOREP_LIBWRAP_WRAPPER( clGetHostTimer )( cl_device_id device,
                                          cl_ulong*    hostTimestamp )
{
    SCOREP_OPENCL_FUNC_ENTER( clGetHostTimer );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_int ret = SCOREP_LIBWRAP_ORIGINAL( clGetHostTimer )( device, hostTimestamp );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clGetHostTimer );

    return ret;
}



cl_int
SCOREP_LIBWRAP_WRAPPER( clGetDeviceAndHostTimer )( cl_device_id device,
                                                   cl_ulong*    deviceTimestamp,
                                                   cl_ulong*    hostTimestamp )
{
    SCOREP_OPENCL_FUNC_ENTER( clGetDeviceAndHostTimer );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_int ret = SCOREP_LIBWRAP_ORIGINAL( clGetDeviceAndHostTimer )(
        device, deviceTimestamp, hostTimestamp );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clGetDeviceAndHostTimer );

    return ret;
}



cl_int
SCOREP_LIBWRAP_WRAPPER( clEnqueueSVMMigrateMem )( cl_command_queue       commandQueue,
                                                  cl_uint                numSvmPointers,
                                                  const void**           svmPointers,
                                                  const size_t*          sizes,
                                                  cl_mem_migration_flags flags,
                                                  cl_uint                numEventsInWaitList,
                                                  const cl_event*        eventWaitList,
                                                  cl_event*              event )
{
    SCOREP_OPENCL_FUNC_ENTER( clEnqueueSVMMigrateMem );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_int ret = SCOREP_LIBWRAP_ORIGINAL( clEnqueueSVMMigrateMem )(
        commandQueue, numSvmPointers, svmPointers, sizes, flags,
        numEventsInWaitList, eventWaitList, event );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clEnqueueSVMMigrateMem );

    return ret;
}



cl_kernel
SCOREP_LIBWRAP_WRAPPER( clCloneKernel )( cl_kernel sourceKernel,
                                         cl_int*   errCodeRet )
{
    SCOREP_OPENCL_FUNC_ENTER( clCloneKernel );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_kernel ret = SCOREP_LIBWRAP_ORIGINAL( clCloneKernel )( sourceKernel, errCodeRet );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clCloneKernel );

    return ret;
}



cl_int
SCOREP_LIBWRAP_WRAPPER( clSetDefaultDeviceCommandQueue )( cl_context       context,
                                                          cl_device_id     device,
                                                          cl_command_queue commandQueue )
{
    SCOREP_OPENCL_FUNC_ENTER( clSetDefaultDeviceCommandQueue );

    SCOREP_OPENCL_WRAP_ENTER();
    cl_int ret = SCOREP_LIBWRAP_ORIGINAL( clSetDefaultDeviceCommandQueue )(
        context, device, commandQueue );
    SCOREP_OPENCL_WRAP_EXIT();

    SCOREP_OPENCL_FUNC_EXIT( clSetDefaultDeviceCommandQueue );

    return ret;
}
