<metrics>
    <metric>
        <!-- This metric is copied from the summary profile -->
        <disp_name>Time</disp_name>
        <uniq_name>time</uniq_name>
        <dtype>FLOAT</dtype>
        <uom>sec</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#time</url>
        <descr>Total CPU allocation time (includes time allocated for idle threads)</descr>
        <metric type="PREDERIVED_EXCLUSIVE">
            <disp_name>Execution</disp_name>
            <uniq_name>execution</uniq_name>
            <dtype>FLOAT</dtype>
            <uom>sec</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#execution</url>
            <descr>Execution time (does not include time allocated for idle threads)</descr>
            <cubepl>
                ${execution}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
            </cubepl>
            <cubeplinit>
            {
                //--- Global callpath type masks ----------------------------
                // Each type mask field contains one entry for each callpath,
                // by default set to 0.  It is set to 1 if the callpath is of
                // the corresponding category.

                // Generic masks
                global(execution);
                global(overhead);

                // MPI-specific masks
                global(mpi);
                global(mpi_sync_collective);
                global(mpi_rma_sync_active);
                global(mpi_rma_sync_passive);
                global(mpi_point2point);
                global(mpi_collective);
                global(mpi_rma_communication);
                global(mpi_io);
                global(mpi_io_individual);
                global(mpi_file_irops);
                global(mpi_file_iwops);
                global(mpi_io_collective);
                global(mpi_file_crops);
                global(mpi_file_cwops);
                global(mpi_init_exit);
                global(syncs_fence);
                global(syncs_gats_access);
                global(syncs_gats_exposure);
                global(syncs_locks);
                global(comms_rma_puts);
                global(comms_rma_gets);

                // OpenMP-specific masks
                global(omp_flush);
                global(omp_management);
                global(omp_ebarrier);
                global(omp_ibarrier);
                global(omp_taskwait);
                global(omp_critical);
                global(omp_lock_api);
                global(omp_ordered);

                // Pthread-specific masks
                global(pthread_management);
                global(pthread_mutex_api);
                global(pthread_cond_api);

                // OpenCL-specific masks
                global(opencl);
                global(opencl_setup);
                global(opencl_comm);
                global(opencl_sync);
                global(opencl_kernel_launches);
                global(opencl_kernel_executions);

                // CUDA-specific masks
                global(cuda);
                global(cuda_setup);
                global(cuda_comm);
                global(cuda_sync);
                global(cuda_kernel_launches);
                global(cuda_kernel_executions);

                //--- Paradigm flags ----------------------------------------
                // These flags indicate which of the supported parallel
                // programming paradigms are used in the experiment to
                // later enable/disable the corresponding metric subtrees.

                ${includesMPI}     = 0;
                ${includesOpenMP}  = 0;
                ${includesPthread} = 0;
                ${includesOpenCL}  = 0;
                ${includesCUDA}  = 0;


                //--- Callpath categorization -------------------------------
                // Each callpath is categorized by type and its global type
                // mask entry is set accordingly.

                ${i} = 0;
                while ( ${i} < ${cube::#callpaths} )
                {
                    ${regionid} = ${cube::callpath::calleeid}[${i}];
                    ${name}     = ${cube::region::name}[${regionid}];
                    ${paradigm} = ${cube::region::paradigm}[${regionid}];
                    ${role}     = ${cube::region::role}[${regionid}];

                    // Default: all callpaths are considered to be execution
                    ${execution}[${i}] = 1;


                    //--- MPI-specific categorization ---
                    if ( ${paradigm} eq "mpi" )
                    {

                        ${includesMPI} = 1;
                        
                        // Without compiler instrumentation artificial PARALLEL region
                        // created with MPI paradigm which should be
                        // filtered out from MPI 
                        if ( not ( ${name} =~ /^PARALLEL/ ) )
                        {
                            ${mpi}[${i}]   = 1;
                        };

                        if ( ${role} eq "barrier" )
                        {
                            ${mpi_sync_collective}[${i}] = 1;
                        };

                        if ( ${name} =~ /^MPI_Win_(post|wait|start|complete|fence)$/ )
                        {
                            ${mpi_rma_sync_active}[${i}] = 1;

                            if ( ${name} eq "MPI_Win_fence" )
                            {
                                ${syncs_fence}[${i}] = 1;
                            };

                            if ( ${name} eq "MPI_Win_start" )
                            {
                                ${syncs_gats_access}[${i}] = 1;
                            };

                            if ( ${name} eq "MPI_Win_post" )
                            {
                                ${syncs_gats_exposure}[${i}] = 1;
                            };
                        };

                        if ( ${name} =~ /^MPI_Win_(lock|unlock)$/ )
                        {
                            ${mpi_rma_sync_passive}[${i}] = 1;

                            if ( ${name} eq "MPI_Win_lock" )
                            {
                                ${syncs_locks}[${i}] = 1;
                            };
                        };

                        if (
                            ( ${role} eq "point2point" )
                            or
                            ( ${name} =~ /^MPI_.*(buffer|cancel|get_count|request)/ )
                           )
                        {
                            ${mpi_point2point}[${i}] = 1;
                        };

                        if ( ${role} =~ /^(one2all|all2one|all2all|other collective)$/ )
                        {
                            ${mpi_collective}[${i}] = 1;
                        };

                        if ( ${name} =~ /^MPI_(Put|Get|Accumulate)$/ )
                        {
                            ${mpi_rma_communication}[${i}] = 1;

                            if ( ${name} eq "MPI_Get" )
                            {
                                ${comms_rma_gets}[${i}] = 1;
                            }
                            else
                            {
                                ${comms_rma_puts}[${i}] = 1;
                            };
                        };

                        if ( ${name} =~ /^MPI_File/ )
                        {
                            ${mpi_io}[${i}] = 1;

                            if (
                                not ( ${name} =~ /^MPI_File_set_err/ )
                                and
                                (
                                    ( ${name} =~ /^MPI_File_(open|close|preallocate|seek_shared|sync)$/ )
                                    or
                                    ( ${name} =~ /^MPI_File.*_(all|ordered|set)/ )
                                )
                               )
                            {
                                ${mpi_io_collective}[${i}] = 1;

                                if ( ${name} =~ /^MPI_File_read.*_(all|ordered)$/ )
                                {
                                    ${mpi_file_crops}[${i}] = 1;
                                };

                                if ( ${name} =~ /^MPI_File_write.*_(all|ordered)$/ )
                                {
                                    ${mpi_file_cwops}[${i}] = 1;
                                };
                            }
                            else
                            {
                                ${mpi_io_individual}[${i}] = 1;

                                if ( ${name} =~ /^MPI_File.*read(_at|_shared)?$/ )
                                {
                                    ${mpi_file_irops}[${i}] = 1;
                                };

                                if ( ${name} =~ /^MPI_File.*write(_at|_shared)?$/ )
                                {
                                    ${mpi_file_iwops}[${i}] = 1;
                                };
                            };
                        };

                        if ( ${name} =~ /^MPI_(Init|Init_thread|Finalize)$/ )
                        {
                            ${mpi_init_exit}[${i}] = 1;
                        };
                    }
                    elseif ( ${paradigm} eq "openmp" )
                    {
                        ${includesOpenMP} = 1;

                        if ( ${role} eq "flush" )
                        {
                            ${omp_flush}[${i}] = 1;
                        };

                        if ( ${role} eq "parallel" )
                        {
                            ${omp_management}[${i}] = 1;
                        };

                        if ( ${role} eq "barrier" )
                        {
                            ${omp_ebarrier}[${i}] = 1;
                        };

                        if ( ${role} eq "implicit barrier" )
                        {
                            ${omp_ibarrier}[${i}] = 1;
                        };

                        if ( ${role} eq "task wait" )
                        {
                            ${omp_taskwait}[${i}] = 1;
                        };

                        if ( ( ${role} eq "atomic" ) or ( ${role} eq "critical" ) )
                        {
                            ${omp_critical}[${i}] = 1;
                        };

                        if ( ${name} =~ /^omp_(destroy|init|set|test|unset)(_nest){0,1}_lock$/ )
                        {
                            ${omp_lock_api}[${i}] = 1;
                        };

                        if ( ${role} eq "ordered" )
                        {
                            ${omp_ordered}[${i}] = 1;
                        };
                    }
                    elseif ( ${paradigm} eq "pthread" )
                    {
                        ${includesPthread} = 1;

                        if ( ${name} =~ /^pthread_(create|join|exit|abort|cancel|detach)$/ )
                        {
                            ${pthread_management}[${i}] = 1;
                        };

                        if ( ${name} =~ /^pthread_mutex_*/ )
                        {
                            ${pthread_mutex_api}[${i}] = 1;
                        };

                        if ( ${name} =~ /^pthread_cond_*/ )
                        {
                            ${pthread_cond_api}[${i}] = 1;
                        };
                    }
                    elseif ( ${paradigm} eq "opencl" )
                    {
                        ${includesOpenCL} = 1;

                        if ( not ( ${name} eq "BUFFER FLUSH" ) )
                        {
                            ${opencl}[${i}] = 1;
                        };

                        if ( ${name} =~ /^cl(GetPlatformIDs|GetPlatformInfo|GetDeviceIDs|GetDeviceInfo|CreateSubDevices|RetainDevice|ReleaseDevice|CreateContext|CreateContextFromType|RetainContext|ReleaseContext|GetContextInfo|CreateProgramWithSource|CreateProgramWithBinary|RetainProgram|ReleaseProgram|GetProgramInfo|GetProgramBuildInfo|CreateKernelsInProgram|CreateProgramWithBuildInKernels|BuildProgram|CompileProgram|LinkProgram|CreateKernel|Retainkernel|ReleaseKernel|GetEventInfo|RetainEvent|ReleaseEvent|GetEventProfilingInfo|CreateUserInfo|SetUserEventStatus|SetEventCallback|CreateCommandQueue|CreateCommandQueueWithProperties|RetainCommandQueue|ReleaseCommandQueue|GetCommandQueueInfo|SetCommandQueueProperty|GetKernelInfo|GetKernelWorkGroupInfo|GetKernelArgInfo|Flush|UnloadCompiler|UnloadPlatformCompiler|GetExtensionFunctionAddress|GetExtensionFunctionAddressForPlatform|SetMemObjectDestructorCallback|GetMemObjectInfo|GetPipeInfo|GetSupportedImageFormats|GetImageInfo|GetSamplerInfo)$/ )
                        {
                            ${opencl_setup}[${i}] = 1;
                        };

                        if ( ${name} =~ /^cl(CreateBuffer|CreateSubBuffer|EnqueueReadBuffer|EnqueueReadBufferRect|EnqueueWriteBuffer|EnqueueWriteBufferRect|EnqueueFillBuffer|EnqueueCopyBuffer|EnqueueCopyBufferRect|EnqueueMapBuffer|EnqueueUnmapMemObjectCopy|EnqueueMigrateMemObjects|CreatePipe|SVMAlloc|SVMFree|EnqueueSVMFree|EnqueueSVMMemcpy|EnqueueSVMMemFill|EnqueueSVMMap|EnqueueSVMUnmap|CreateImage|CreateImage2D|CreateImage3D|EnqueueReadImage|EnqueueWriteImage|EnqueueCopyImage|EnqueueCopyImageToBuffer|EnqueueCopyBufferToImage|EnqueueMapImage|EnqueueFillImage|CreateSamplerWithProperties|CreateSampler|ReleaseSampler|RetainSampler|RetainMemObject|ReleaseMemObject)$/ )
                        {
                            ${opencl_comm}[${i}] = 1;
                        };

                        if ( ${name} =~ /^(clFinish|clWaitForEvents|clEnqueueWaitForEvents|clEnqueueBarrier|clEnqueueMarker|clEnqueueMarkerWithWaitList|clEnqueueBarrierWithWaitList)$/ )
                        {
                            ${opencl_sync}[${i}] = 1;
                        };

                        if ( ${name} eq "WAIT FOR COMMAND QUEUE" )
                        {
                            ${opencl_sync}[${i}] = 1;
                        };

                        if ( ${name} =~ /^cl(SetKernelArg|SetKernelArgSVMPointer|SetKernelExecInfo|EnqueueNDRangeKernel|EnqueueTask)$/ )
                        {
                            ${opencl_kernel_launches}[${i}] = 1;
                        };

                        if ( ${cube::region::mod}[${regionid}] seq "OPENCL_KERNEL" )
                        {
                            ${opencl_kernel_executions}[${i}] = 1;
                        };

                        if ( ${name} eq "BUFFER FLUSH" )
                        {
                            ${opencl}[${i}] = 0;
                            ${execution}[${i}] = 0;
                            ${overhead}[${i}]  = 1;
                        };
                    }
                    elseif ( ${paradigm} eq "cuda" )
                    {
                        ${includesCUDA} = 1;


                        if ( not ( ${name} eq "BUFFER FLUSH" ) )
                        {
                            ${cuda}[${i}] = 1;
                        };

                        // CUDA Runtime API
                        // Initialization and finalization
                        if ( ${name} =~ /^cuda(ChooseDevice|DeviceGetAttribute|DeviceGetByPCIBusId|DeviceGetCacheConfig|DeviceGetLimit|DeviceGetPCIBusId|DeviceGetSharedMemConfig|DeviceGetStreamPriorityRange|DeviceReset|DeviceSetCacheConfig|DeviceSetLimit|DeviceSetSharedMemConfig|GetDevice|GetDeviceCount|GetDeviceFlags|GetDeviceProperties|SetDevice|SetDeviceFlags|SetValidDevices|ThreadExit|ThreadGetCacheConfig|ThreadGetLimit|ThreadSetCacheConfig|ThreadSetLimit|StreamCreate|StreamCreateWithFlags|StreamCreateWithPriority|StreamDestroy|EventCreate|EventCreateWithFlags|EventDestroy|FuncSetCacheConfig)$/ )
                        {
                            ${cuda_setup}[${i}] = 1;
                        };

                        // Data transfer
                        if ( ${name} =~ /^cuda(Free|FreeArray|FreeHost|FreeMipmappedArray|HostAlloc|Malloc|Malloc3D|Malloc3DArray|MallocArray|MallocHost|MallocManaged|MallocMipmappedArray|MallocPitch|Memcpy|Memcpy2D|Memcpy2DArrayToArray|Memcpy2DAsync|Memcpy2DFromArray|Memcpy2DFromArrayAsync|Memcpy2DToArray|Memcpy2DToArrayAsync|Memcpy3D|Memcpy3DAsync|MemcpyPeer|MemcpyPeerAsync|MemcpyArrayToArray|MemcpyAsync|MemcpyFromArray|MemcpyFromArrayAsync|MemcpyFromSymbol|MemcpyFromSymbolAsync|MemcpyFromPeer|MemcpyFromPeerAsync|MemcpyToArray|MemcpyToArrayAsync|MemcpyToSymbol|MemcpyToSymbolAsync)$/ )
                        {
                            ${cuda_comm}[${i}] = 1;
                        };

                        // Synchronization
                        if ( ${name} =~ /^cuda(StreamWaitEvent|StreamSynchronize|EventSynchronize|DeviceSynchronize|ThreadSynchronize)$/ )
                        {
                            ${cuda_sync}[${i}] = 1;
                        };

                        // Kernel launch
                        if ( ${name} =~ /^cuda(LaunchKernel|Launch|GetParameterBufferV2|Memset|Memset2D|Memset2DAsync|Memset3D|Memset3DAsync|MemsetAsync)$/ )
                        {
                            ${cuda_kernel_launches}[${i}] = 1;
                        };

                        // CUDA Driver API
                        // Initialization and finalization
                        if ( ${name} =~ /^cu(Init|DeviceGet|DeviceGetAttribute|DeviceGetCount|DeviceGetName|DeviceTotalMem|DeviceComputeCapability|DeviceGetProperties|DevicePrimaryCtxGetState|DevicePrimaryCtxRelease|DevicePrimaryCtxReset|DevicePrimaryCtxRetain|DevicePrimaryCtxSetFlags|CtxCrate|CtxDestroy|CtxGetApiVersion|CtxGetCacheConfig|CtxGetCurrent|CtxGetDevice|CtxGetFlags|CtxGetLimit|CtxGetSharedMemConfig|CtxGetStreamPriorityRange|CtxPopCurrent|CtxPushCurrent|CtxSetCacheConfig|CtxSetCurrent|CtxSetLimit|CtxSetSharedMemConfig|CtxAttach|CtxDetach|LinkCreate|LinkDestroy|EventCreate|EventDestroy|StreamCreate|StreamCreateWithPriority|StreamDestroy|MemFree|MemFreeHost|MemHostAlloc)$/ )
                        {
                            ${cuda_setup}[${i}] = 1;
                        };

                        // Data transfer
                        if ( ${name} =~ /^cu(Array3DCreate|ArrayCreate|ArrayDestroy|MemAlloc|MemAllocHost|MemAllocManaged|MemAllocPitch|Memcpy|Memcpy2D|Memcpy2DAsync|Memcpy2DUnaligned|Memcpy3D|Memcpy3DAsync|Memcpy3DPeer|Memcpy3DPeerAsync|MemcpyAsync|MemcpyAtoA|MemcpyAtoD|MemcpyAtoH|MemcpyAtoHAsync|MemcpyDtoA|MemcpyDtoD|MemcpyDtoDAsync|MemcpyDtoH|MemcpyDtoHAsync|MemcpyHtoA|MemcpyHtoAAsync|MemcpyHtoD|MemcpyHtoDAsync|MemcpyPeer|MemcpyPeerAsync|MemsetD16|MemsetD16Async|MemsetD2D16|MemsetD2D16Async|MemsetD2D32|MemsetD2D32Async|MemsetD2D8|MemsetD2D8Async|MemsetD32|MemsetD32Async|MemsetD8|MemsetD8Async|MipmappedArrayCreate|MipmappedArrayDestroy)$/ )
                        {
                            ${cuda_comm}[${i}] = 1;
                        };

                        // Synchronization
                        if ( ${name} =~ /^cu(CtxSynchronize|StreamWaitEvent|StreamSynchronize|EventSynchronize)$/ )
                        {
                            ${cuda_sync}[${i}] = 1;
                        };

                        // Kernel launch
                        if ( ${name} =~ /^cu(LaunchKernel|Launch|LaunchGrid|LaunchGridAsync)$/ )
                        {
                            ${cuda_kernel_launches}[${i}] = 1;
                        };

                        if ( ${name} eq "DEVICE SYNCHRONIZE" )
                        {
                            ${cuda_sync}[${i}] = 1;
                        };

                        if ( ${cube::region::mod}[${regionid}] seq "CUDA_KERNEL" )
                        {
                            ${cuda_kernel_executions}[${i}] = 1;
                        };

                        if ( ${name} eq "BUFFER FLUSH" )
                        {
                            ${cuda}[${i}] = 0;
                            ${execution}[${i}] = 0;
                            ${overhead}[${i}]  = 1;
                        };
                    }
                    elseif (
                        ( ${paradigm} eq "measurement" )
                        or
                        ( ${name} eq "BUFFER FLUSH" )
                        or 
                        ( ${name} eq "MEASUREMENT OFF" )
                        or
                        ( ( ${paradigm} eq "thread-fork-join" ) and ( ${name} eq "TASKS" ) )
                       )
                    {
                        ${execution}[${i}] = 0;
                        ${overhead}[${i}]  = 1;
                    };

                    ${i} = ${i} + 1;
                };


                //--- Disable unused metric subtrees ------------------------

                if ( ${includesMPI} == 0 )
                {
                    cube::metric::set::mpi("value", "VOID");
                };
                if ( ${includesOpenMP} == 0 )
                {
                    cube::metric::set::omp_time("value", "VOID");
                };
                if ( ${includesPthread} == 0 )
                {
                    cube::metric::set::pthread_time("value", "VOID");
                };
                if ( ${includesOpenCL} == 0 )
                {
                    cube::metric::set::opencl_time("value", "VOID");
                    cube::metric::set::opencl_kernel_executions("value", "VOID");
                };
                if ( ${includesCUDA} == 0 )
                {
                    cube::metric::set::cuda_time("value", "VOID");
                    cube::metric::set::cuda_kernel_executions("value", "VOID");
                };

                return 0;
            }
            </cubeplinit>
            <metric type="POSTDERIVED">
                <disp_name>Computation</disp_name>
                <uniq_name>comp</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#comp</url>
                <descr>Time spent on computation</descr>
                <cubepl>
                    metric::execution() - metric::mpi() - metric::omp_time() - metric::pthread_time() - metric::opencl_time() - metric::cuda_time()
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>OpenCL kernels</disp_name>
                    <uniq_name>opencl_kernel_executions</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_kernel_executions</url>
                    <descr>Time spend in execution of OpenCL kernels</descr>
                    <cubepl>
                        ${opencl_kernel_executions}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>CUDA kernels</disp_name>
                    <uniq_name>cuda_kernel_executions</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_kernel_executions</url>
                    <descr>Time spend in execution of CUDA kernels</descr>
                    <cubepl>
                        ${cuda_kernel_executions}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>MPI</disp_name>
                <uniq_name>mpi</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi</url>
                <descr>Time spent in MPI calls</descr>
                <cubepl>
                    ${mpi}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                </cubepl>
                <metric type="POSTDERIVED">
                    <disp_name>Synchronization</disp_name>
                    <uniq_name>mpi_synchronization</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_synchronization</url>
                    <descr>Time spent in MPI synchronization calls</descr>
                    <cubepl>
                        metric::mpi_sync_collective() + metric::mpi_rma_synchronization()
                    </cubepl>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Collective</disp_name>
                        <uniq_name>mpi_sync_collective</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_sync_collective</url>
                        <descr>Time spent on MPI barriers</descr>
                        <cubepl>
                            ${mpi_sync_collective}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="POSTDERIVED">
                        <disp_name>Remote Memory Access</disp_name>
                        <uniq_name>mpi_rma_synchronization</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_rma_synchronization</url>
                        <descr>Time spent in MPI RMA synchronization calls</descr>
                        <cubepl>
                            metric::mpi_rma_sync_active() + metric::mpi_rma_sync_passive()
                        </cubepl>
                        <metric type="PREDERIVED_EXCLUSIVE">
                            <disp_name>Active Target</disp_name>
                            <uniq_name>mpi_rma_sync_active</uniq_name>
                            <dtype>FLOAT</dtype>
                            <uom>sec</uom>
                            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_rma_sync_active</url>
                            <descr>Time spent in MPI RMA active target synchronization calls</descr>
                            <cubepl>
                                ${mpi_rma_sync_active}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                            </cubepl>
                        </metric>
                        <metric type="PREDERIVED_EXCLUSIVE">
                            <disp_name>Passive Target</disp_name>
                            <uniq_name>mpi_rma_sync_passive</uniq_name>
                            <dtype>FLOAT</dtype>
                            <uom>sec</uom>
                            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_rma_sync_passive</url>
                            <descr>Time spent in MPI RMA passive target synchronization calls</descr>
                            <cubepl>
                                ${mpi_rma_sync_passive}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                            </cubepl>
                        </metric>
                    </metric>
                </metric>
                <metric type="POSTDERIVED">
                    <disp_name>Communication</disp_name>
                    <uniq_name>mpi_communication</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_communication</url>
                    <descr>Time spent in MPI communication calls</descr>
                    <cubepl>
                        metric::mpi_point2point() + metric::mpi_collective() + metric::mpi_rma_communication()
                    </cubepl>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Point-to-point</disp_name>
                        <uniq_name>mpi_point2point</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_point2point</url>
                        <descr>MPI point-to-point communication</descr>
                        <cubepl>
                            ${mpi_point2point}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Collective</disp_name>
                        <uniq_name>mpi_collective</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_collective</url>
                        <descr>MPI collective communication</descr>
                        <cubepl>
                            ${mpi_collective}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Remote Memory Access</disp_name>
                        <uniq_name>mpi_rma_communication</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_rma_communication</url>
                        <descr>MPI remote memory access communication</descr>
                        <cubepl>
                            ${mpi_rma_communication}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>File I/O</disp_name>
                    <uniq_name>mpi_io</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_io</url>
                    <descr>Time spent in MPI file I/O calls</descr>
                    <cubepl>
                        ${mpi_io}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Collective</disp_name>
                        <uniq_name>mpi_io_collective</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_io_collective</url>
                        <descr>Time spent in collective MPI file I/O calls</descr>
                        <cubepl>
                            ${mpi_io_collective}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Init/Exit</disp_name>
                    <uniq_name>mpi_init_exit</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_init_exit</url>
                    <descr>Time spent in MPI initialization calls</descr>
                    <cubepl>
                        ${mpi_init_exit}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
            </metric>
            <metric type="POSTDERIVED">
                <disp_name>OMP</disp_name>
                <uniq_name>omp_time</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_time</url>
                <descr>Time spent in the OpenMP run-time system and API</descr>
                <cubepl>
                    metric::omp_flush() + metric::omp_management() + metric::omp_synchronization()
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Flush</disp_name>
                    <uniq_name>omp_flush</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_flush</url>
                    <descr>Time spent in the OpenMP flush directives</descr>
                    <cubepl>
                        ${omp_flush}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Management</disp_name>
                    <uniq_name>omp_management</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_management</url>
                    <descr>Time needed to start up and shut down team of threads</descr>
                    <cubepl>
                        ${omp_management}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="POSTDERIVED">
                    <disp_name>Synchronization</disp_name>
                    <uniq_name>omp_synchronization</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_synchronization</url>
                    <descr>Time spent on OpenMP synchronization</descr>
                    <cubepl>
                        metric::omp_barrier() + metric::omp_critical() + metric::omp_lock_api() + metric::omp_ordered()
                    </cubepl>
                    <metric type="POSTDERIVED">
                        <disp_name>Barrier</disp_name>
                        <uniq_name>omp_barrier</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_barrier</url>
                        <descr>OpenMP barrier synchronization</descr>
                        <cubepl>
                            metric::omp_ebarrier() + metric::omp_ibarrier()
                        </cubepl>
                        <metric type="PREDERIVED_EXCLUSIVE">
                            <disp_name>Explicit</disp_name>
                            <uniq_name>omp_ebarrier</uniq_name>
                            <dtype>FLOAT</dtype>
                            <uom>sec</uom>
                            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_ebarrier</url>
                            <descr>Time spent in explicit OpenMP barriers</descr>
                            <cubepl>
                                ${omp_ebarrier}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                            </cubepl>
                        </metric>
                        <metric type="PREDERIVED_EXCLUSIVE">
                            <disp_name>Implicit</disp_name>
                            <uniq_name>omp_ibarrier</uniq_name>
                            <dtype>FLOAT</dtype>
                            <uom>sec</uom>
                            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_ibarrier</url>
                            <descr>Time spent in implicit OpenMP barriers</descr>
                            <cubepl>
                                ${omp_ibarrier}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                            </cubepl>
                        </metric>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Task Wait</disp_name>
                        <uniq_name>omp_taskwait</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_MAJOR@.@PACKAGE_MINOR@.html#omp_taskwait</url>
                        <descr>Time spent waiting for child tasks to finish</descr>
                        <cubepl>
                            ${omp_taskwait}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Critical</disp_name>
                        <uniq_name>omp_critical</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_critical</url>
                        <descr>Time spent in front of a critical section</descr>
                        <cubepl>
                            ${omp_critical}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Lock API</disp_name>
                        <uniq_name>omp_lock_api</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_lock_api</url>
                        <descr>Time spent in OpenMP API calls dealing with locks</descr>
                        <cubepl>
                            ${omp_lock_api}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Ordered</disp_name>
                        <uniq_name>omp_ordered</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_ordered</url>
                        <descr>Time spent in front of an ordered region</descr>
                        <cubepl>
                            ${omp_ordered}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                </metric>
            </metric>
            <metric type="POSTDERIVED">
                <disp_name>Pthread</disp_name>
                <uniq_name>pthread_time</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#pthread_time</url>
                <descr>Time spent in the Pthread run-time system and API</descr>
                <cubepl>
                    metric::pthread_management() + metric::pthread_synchronization()
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Management</disp_name>
                    <uniq_name>pthread_management</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#pthread_management</url>
                    <descr>Time needed to start up and shut down POSIX threads</descr>
                    <cubepl>
                        ${pthread_management}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="POSTDERIVED">
                    <disp_name>Synchronization</disp_name>
                    <uniq_name>pthread_synchronization</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#pthread_synchronization</url>
                    <descr>Time spent on Pthread synchronization</descr>
                    <cubepl>
                        metric::pthread_mutex_api() + metric::pthread_cond_api()
                    </cubepl>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Mutex</disp_name>
                        <uniq_name>pthread_mutex_api</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#pthread_mutex_api</url>
                        <descr>Time spent in Pthread API calls dealing with mutexes</descr>
                        <cubepl>
                            ${pthread_mutex_api}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                    <metric type="PREDERIVED_EXCLUSIVE">
                        <disp_name>Conditional</disp_name>
                        <uniq_name>pthread_cond_api</uniq_name>
                        <dtype>FLOAT</dtype>
                        <uom>sec</uom>
                        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#pthread_cond_api</url>
                        <descr>Time spent in Pthread API calls dealing with conditional operations</descr>
                        <cubepl>
                            ${pthread_cond_api}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                        </cubepl>
                    </metric>
                </metric>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>OpenCL</disp_name>
                <uniq_name>opencl_time</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_time</url>
                <descr>Time spent in the OpenCL run-time system, API and on device</descr>
                <cubepl>
                    ${opencl}[${calculation::callpath::id}] * ( metric::time(e) - metric::opencl_kernel_executions(e) -  metric::omp_idle_threads(e) )
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Initialization and finalization</disp_name>
                    <uniq_name>opencl_setup</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_setup</url>
                    <descr>Time needed to initialize and finalize OpenCL and OpenCL kernels</descr>
                    <cubepl>
                        ${opencl_setup}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Memory management</disp_name>
                    <uniq_name>opencl_comm</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_comm</url>
                    <descr>Time spent on memory management including data transfer from host to device and vice versa</descr>
                    <cubepl>
                        ${opencl_comm}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Synchronization</disp_name>
                    <uniq_name>opencl_sync</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_sync</url>
                    <descr>Time spent on OpenCL synchronization</descr>
                    <cubepl>
                        ${opencl_sync}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Kernel launches</disp_name>
                    <uniq_name>opencl_kernel_launches</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#opencl_kernel_launches</url>
                    <descr>Time needed to launch OpenCL kernels</descr>
                    <cubepl>
                        ${opencl_kernel_launches}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>CUDA</disp_name>
                <uniq_name>cuda_time</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_time</url>
                <descr>Time spent in the CUDA run-time system, API and on device</descr>
                <cubepl>
                    ${cuda}[${calculation::callpath::id}] * ( metric::time(e) - metric::cuda_kernel_executions(e) -  metric::omp_idle_threads(e) )
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Initialization and Finalization</disp_name>
                    <uniq_name>cuda_setup</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_setup</url>
                    <descr>Time needed to initialize and finalize CUDA and CUDA kernels</descr>
                    <cubepl>
                        ${cuda_setup}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Memory management</disp_name>
                    <uniq_name>cuda_comm</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_comm</url>
                    <descr>Time spent on memory management including data transfer from host to device and vice versa</descr>
                    <cubepl>
                        ${cuda_comm}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Synchronization</disp_name>
                    <uniq_name>cuda_sync</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_sync</url>
                    <descr>Time spent on CUDA synchronization</descr>
                    <cubepl>
                        ${cuda_sync}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Kernel launches</disp_name>
                    <uniq_name>cuda_kernel_launches</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#cuda_kernel_launches</url>
                    <descr>Time spent to launch CUDA kernels</descr>
                    <cubepl>
                        ${cuda_kernel_launches}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
                    </cubepl>
                </metric>
            </metric>
        </metric>
        <metric type="PREDERIVED_EXCLUSIVE">
            <disp_name>Overhead</disp_name>
            <uniq_name>overhead</uniq_name>
            <dtype>FLOAT</dtype>
            <uom>sec</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#overhead</url>
            <descr>Time spent performing tasks related to trace generation</descr>
            <cubepl>
                ${overhead}[${calculation::callpath::id}] * ( metric::time(e) - metric::omp_idle_threads(e) )
            </cubepl>
        </metric>
        <metric>
            <!-- This metric is still hard-coded in the Cube remapper -->
            <disp_name>Idle threads</disp_name>
            <uniq_name>omp_idle_threads</uniq_name>
            <dtype>FLOAT</dtype>
            <uom>sec</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_idle_threads</url>
            <descr>Unused CPU reservation time</descr>
            <metric>
                <!-- This metric is still hard-coded in the Cube remapper -->
                <disp_name>Limited parallelism</disp_name>
                <uniq_name>omp_limited_parallelism</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#omp_limited_parallelism</url>
                <descr>Unused CPU reservation time in parallel regions due to limited parallelism</descr>
            </metric>
        </metric>
    </metric>
    <metric>
        <!-- This metric is copied from the summary profile -->
        <disp_name>Visits</disp_name>
        <uniq_name>visits</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>occ</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#visits</url>
        <descr>Number of visits</descr>
    </metric>
    <metric type="POSTDERIVED">
        <disp_name>Synchronizations</disp_name>
        <uniq_name>syncs</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>occ</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs</url>
        <descr>Number of synchronization operations</descr>
        <cubepl>
            metric::syncs_rma()
        </cubepl>
        <metric type="POSTDERIVED">
            <disp_name>Remote Memory Access</disp_name>
            <uniq_name>syncs_rma</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>occ</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_rma</url>
            <descr>Number of Remote Memory Access synchronizations</descr>
            <cubepl>
                metric::syncs_fence() + metric::syncs_gats()  + metric::syncs_locks()
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Fences</disp_name>
                <uniq_name>syncs_fence</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_fence</url>
                <descr>Number of fence synchronizations</descr>
                <cubepl>
                    ${syncs_fence}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
            <metric type="POSTDERIVED">
                <disp_name>GATS Epochs</disp_name>
                <uniq_name>syncs_gats</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_gats</url>
                <descr>Number of GATS epochs</descr>
                <cubepl>
                    metric::syncs_gats_access() + metric::syncs_gats_exposure()
                </cubepl>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Access Epochs</disp_name>
                    <uniq_name>syncs_gats_access</uniq_name>
                    <dtype>INTEGER</dtype>
                    <uom>occ</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_gats_access</url>
                    <descr>Number of access epochs</descr>
                    <cubepl>
                        ${syncs_gats_access}[${calculation::callpath::id}] * metric::visits(e)
                    </cubepl>
                </metric>
                <metric type="PREDERIVED_EXCLUSIVE">
                    <disp_name>Exposure Epochs</disp_name>
                    <uniq_name>syncs_gats_exposure</uniq_name>
                    <dtype>INTEGER</dtype>
                    <uom>occ</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_gats_exposure</url>
                    <descr>Number of exposure epochs</descr>
                    <cubepl>
                        ${syncs_gats_exposure}[${calculation::callpath::id}] * metric::visits(e)
                    </cubepl>
                </metric>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Lock Epochs</disp_name>
                <uniq_name>syncs_locks</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#syncs_locks</url>
                <descr>Number of lock epochs</descr>
                <cubepl>
                    ${syncs_locks}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
        </metric>
    </metric>
    <metric type="POSTDERIVED">
        <disp_name>Communications</disp_name>
        <uniq_name>comms</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>occ</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#comms</url>
        <descr>Number of communication operations</descr>
        <cubepl>
            metric::comms_rma()
        </cubepl>
        <metric type="POSTDERIVED">
            <disp_name>Remote Memory Access</disp_name>
            <uniq_name>comms_rma</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>occ</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#comms_rma</url>
            <descr>Number of remote memory access operations</descr>
            <cubepl>
                metric::comms_rma_gets() + metric::comms_rma_puts()
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Puts</disp_name>
                <uniq_name>comms_rma_puts</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#comms_rma_puts</url>
                <descr>Number of RMA put and accumulate operations</descr>
                <cubepl>
                    ${comms_rma_puts}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Gets</disp_name>
                <uniq_name>comms_rma_gets</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#comms_rma_gets</url>
                <descr>Number of RMA get operations</descr>
                <cubepl>
                    ${comms_rma_gets}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
        </metric>
    </metric>
    <metric type="POSTDERIVED">
        <disp_name>Bytes transferred</disp_name>
        <uniq_name>bytes</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>bytes</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes</url>
        <descr>Number of bytes transferred</descr>
        <cubepl>
            metric::bytes_p2p() + metric::bytes_coll() + metric::bytes_rma()
        </cubepl>
        <metric type="POSTDERIVED">
            <disp_name>Point-to-point</disp_name>
            <uniq_name>bytes_p2p</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>bytes</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_p2p</url>
            <descr>Number of bytes transferred in point-to-point communication operations</descr>
            <cubepl>
                metric::bytes_sent_p2p() + metric::bytes_received_p2p()
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Sent</disp_name>
                <uniq_name>bytes_sent_p2p</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_sent_p2p</url>
                <descr>Number of bytes sent in point-to-point communication operations</descr>
                <cubepl>
                    ${mpi_point2point}[${calculation::callpath::id}] * metric::bytes_sent(e)
                </cubepl>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Received</disp_name>
                <uniq_name>bytes_received_p2p</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_received_p2p</url>
                <descr>Number of bytes received in point-to-point communication operations</descr>
                <cubepl>
                    ${mpi_point2point}[${calculation::callpath::id}] * metric::bytes_received(e)
                </cubepl>
            </metric>
        </metric>
        <metric type="POSTDERIVED">
            <disp_name>Collective</disp_name>
            <uniq_name>bytes_coll</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>bytes</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_p2p</url>
            <descr>Number of bytes transferred in collective communication operations</descr>
            <cubepl>
                metric::bytes_sent_coll() + metric::bytes_received_coll()
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Outgoing</disp_name>
                <uniq_name>bytes_sent_coll</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_sent_coll</url>
                <descr>Number of bytes sent in collective communication operations</descr>
                <cubepl>
                    ${mpi_collective}[${calculation::callpath::id}] * metric::bytes_sent(e)
                </cubepl>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Incoming</disp_name>
                <uniq_name>bytes_received_coll</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_received_coll</url>
                <descr>Number of bytes received in collective communication operations</descr>
                <cubepl>
                    ${mpi_collective}[${calculation::callpath::id}] * metric::bytes_received(e)
                </cubepl>
            </metric>
        </metric>
        <metric type="POSTDERIVED">
            <disp_name>Remote Memory Access</disp_name>
            <uniq_name>bytes_rma</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>bytes</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_MAJOR@.@PACKAGE_MINOR@.html#bytes_rma</url>
            <descr>Number of bytes transferred in remote memory access operations</descr>
            <cubepl>
                metric::bytes_put() + metric::bytes_get()
            </cubepl>
            <metric>
                <!-- This metric is copied from the trace analysis -->
                <disp_name>Sent</disp_name>
                <uniq_name>bytes_put</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_MAJOR@.@PACKAGE_MINOR@.html#bytes_put</url>
                <descr>Number of bytes sent in remote memory access operations</descr>
            </metric>
            <metric>
                <!-- This metric is copied from the trace analysis -->
                <disp_name>Received</disp_name>
                <uniq_name>bytes_get</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>bytes</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_MAJOR@.@PACKAGE_MINOR@.html#bytes_get</url>
                <descr>Number of bytes received in remote memory access operations</descr>
            </metric>
        </metric>
    </metric>
    <metric type="PREDERIVED_EXCLUSIVE">
        <disp_name>MPI file operations</disp_name>
        <uniq_name>mpi_file_ops</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>occ</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_ops</url>
        <descr>Number of MPI file operations</descr>
        <cubepl>
            ${mpi_io}[${calculation::callpath::id}] * metric::visits(e)
        </cubepl>
        <metric type="PREDERIVED_EXCLUSIVE">
            <disp_name>Individual</disp_name>
            <uniq_name>mpi_file_iops</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>occ</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_iops</url>
            <descr>Number of individual MPI file operations</descr>
            <cubepl>
                ${mpi_io_individual}[${calculation::callpath::id}] * metric::visits(e)
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Reads</disp_name>
                <uniq_name>mpi_file_irops</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_irops</url>
                <descr>Number of individual MPI file read operations</descr>
                <cubepl>
                    ${mpi_file_irops}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Writes</disp_name>
                <uniq_name>mpi_file_iwops</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_iwops</url>
                <descr>Number of individual MPI file write operations</descr>
                <cubepl>
                    ${mpi_file_iwops}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
        </metric>
        <metric type="PREDERIVED_EXCLUSIVE">
            <disp_name>Collective</disp_name>
            <uniq_name>mpi_file_cops</uniq_name>
            <dtype>INTEGER</dtype>
            <uom>occ</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_cops</url>
            <descr>Number of collective MPI file operations</descr>
            <cubepl>
                ${mpi_io_collective}[${calculation::callpath::id}] * metric::visits(e)
            </cubepl>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Reads</disp_name>
                <uniq_name>mpi_file_crops</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_crops</url>
                <descr>Number of collective MPI file read operations</descr>
                <cubepl>
                    ${mpi_file_crops}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
            <metric type="PREDERIVED_EXCLUSIVE">
                <disp_name>Writes</disp_name>
                <uniq_name>mpi_file_cwops</uniq_name>
                <dtype>INTEGER</dtype>
                <uom>occ</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#mpi_file_cwops</url>
                <descr>Number of collective MPI file write operations</descr>
                <cubepl>
                    ${mpi_file_cwops}[${calculation::callpath::id}] * metric::visits(e)
                </cubepl>
            </metric>
        </metric>
    </metric>
    <metric>
        <disp_name>Computational imbalance</disp_name>
        <uniq_name>imbalance</uniq_name>
        <dtype>FLOAT</dtype>
        <uom>sec</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance</url>
        <descr>Computational load imbalance heuristic (see Online Description for details)</descr>
        <metric>
            <!-- This metric is still hard-coded in the Cube remapper -->
            <disp_name>Overload</disp_name>
            <uniq_name>imbalance_above</uniq_name>
            <dtype>FLOAT</dtype>
            <uom>sec</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance_above</url>
            <descr>Computational load imbalance heuristic (overload)</descr>
            <metric>
                <!-- This metric is still hard-coded in the Cube remapper -->
                <disp_name>Single participant</disp_name>
                <uniq_name>imbalance_above_single</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance_above_single</url>
                <descr>Computational load imbalance heuristic (single participant)</descr>
            </metric>
        </metric>
        <metric>
            <!-- This metric is still hard-coded in the Cube remapper -->
            <disp_name>Underload</disp_name>
            <uniq_name>imbalance_below</uniq_name>
            <dtype>FLOAT</dtype>
            <uom>sec</uom>
            <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance_below</url>
            <descr>Computational load imbalance heuristic (underload)</descr>
            <metric>
                <!-- This metric is still hard-coded in the Cube remapper -->
                <disp_name>Non-participation</disp_name>
                <uniq_name>imbalance_below_bypass</uniq_name>
                <dtype>FLOAT</dtype>
                <uom>sec</uom>
                <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance_below_bypass</url>
                <descr>Computational load imbalance heuristic (non-participation)</descr>
                <metric>
                    <!-- This metric is still hard-coded in the Cube remapper -->
                    <disp_name>Singularity</disp_name>
                    <uniq_name>imbalance_below_singularity</uniq_name>
                    <dtype>FLOAT</dtype>
                    <uom>sec</uom>
                    <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#imbalance_below_singularity</url>
                    <descr>Computational load imbalance heuristic (non-participation in singularity)</descr>
                </metric>
            </metric>
        </metric>
    </metric>
    <metric viztype="GHOST">
        <disp_name>Bytes sent</disp_name>
        <uniq_name>bytes_sent</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>bytes</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_sent</url>
        <descr>Number of bytes sent in point-to-point communication operations</descr>
    </metric>
    <metric viztype="GHOST">
        <disp_name>Bytes received</disp_name>
        <uniq_name>bytes_received</uniq_name>
        <dtype>INTEGER</dtype>
        <uom>bytes</uom>
        <url>@mirror@scorep_metrics-@PACKAGE_VERSION@.html#bytes_rcvd</url>
        <descr>Number of bytes received in point-to-point communication operations</descr>
    </metric>
</metrics>
