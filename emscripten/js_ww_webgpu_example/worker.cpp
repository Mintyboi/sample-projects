#include <iostream>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

extern "C" {
    extern void jsTransferImageWebGPU();
}


WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapchain;
WGPUTextureFormat swapPref;

WGPURenderPipeline renderPipeline;
WGPUComputePipeline computePipeline;

WGPUBuffer shapeBuf;
WGPUBuffer uniformBuf;
WGPUBuffer particlesBuf[2]  = {0};
WGPUBindGroup particleComputeBindGroups[2] = {0};

static const uint32_t NUM_PARTICLES = 90;
static const uint32_t WORKGROUP_SIZE = 64;

WGPUBindGroupLayout renderBindGroupLayout = nullptr;
WGPUBindGroupLayout computeBindGroupLayout = nullptr;

float const shapeData[] = {
    -0.01f, -0.02f,
    0.01f, -0.02f,
    0.00f, 0.02f,
};

float const uniformData[] = {
    1.0,
    1.0
};


static char const compute_wgsl[] = R"(
    struct Particle {
        pos : vec2<f32>
    }
    struct Particles {
        particles : array<Particle>,
    }

    @binding(0) @group(0) var<storage, read> particlesA : Particles;
    @binding(1) @group(0) var<storage, read_write> particlesB : Particles;

    @compute @workgroup_size(64)
    fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
        var index = GlobalInvocationID.x;
        particlesB.particles[index].pos.y = particlesA.particles[index].pos.y + 0.001;
        if (particlesB.particles[index].pos.y > 0.9) {
            particlesB.particles[index].pos.y = -0.9;
        }
    }
)";

static char const triangle_vert_wgsl[] = R"(
	struct VertexIn {
        @location(0) a_particlePos : vec2<f32>,
        @location(1) a_pos : vec2<f32>
	};
	struct VertexOut {
		@builtin(position) Position : vec4<f32>
	};
    struct Params {
        scale1: f32,
        scale2: f32
    }
    @group(0) @binding(0)
    var<uniform> param: Params;    // A uniform buffer
	@vertex
	fn main(input : VertexIn) -> VertexOut {
        var output : VertexOut;
        output.Position = vec4<f32>(input.a_pos + input.a_particlePos, 0.0, 1.0);
        return output;
	}
)";

static char const triangle_frag_wgsl[] = R"(
	@fragment
	fn main() -> @location(0) vec4<f32> {
		return vec4<f32>(1.0, 0.0, 0.0, 1.0);
	}
)";

static WGPUShaderModule createShader(const char* const code, const char* label = nullptr) {
    WGPUShaderModuleWGSLDescriptor wgsl = {};
    wgsl.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgsl.source = code;
    WGPUShaderModuleDescriptor desc = {};
    desc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgsl);
    desc.label = label;
    return wgpuDeviceCreateShaderModule(device, &desc);
}

static WGPUBuffer createBuffer(const void* data, size_t size, WGPUBufferUsage usage) {
    WGPUBufferDescriptor desc = {};
    desc.usage = WGPUBufferUsage_CopyDst | usage;
    desc.size = size;
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &desc);
    wgpuQueueWriteBuffer(queue, buffer, 0, data, size);
    return buffer;
}

static void createComputePipeline() {

    // compile shaders
    WGPUShaderModule computeMod = createShader(compute_wgsl);

    // create compute pipeline layout
    WGPUBindGroupLayoutEntry bgl_entries[2] = {};
    bgl_entries[0].binding = 0;
    bgl_entries[0].visibility = WGPUShaderStage_Compute;
    bgl_entries[0].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
    bgl_entries[0].buffer.minBindingSize = NUM_PARTICLES * 4;
    bgl_entries[0].sampler = {0};

    bgl_entries[1].binding = 1;
    bgl_entries[1].visibility = WGPUShaderStage_Compute;
    bgl_entries[1].buffer.type = WGPUBufferBindingType_Storage;
    bgl_entries[1].buffer.minBindingSize = NUM_PARTICLES * 4;
    bgl_entries[1].sampler = {0};

    WGPUBindGroupLayoutDescriptor bgl_desc = {};
    bgl_desc.entryCount = 2;
    bgl_desc.entries = bgl_entries;

    computeBindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &bgl_desc);
    assert(computeBindGroupLayout != NULL);

    WGPUPipelineLayoutDescriptor compute_pipeline_layout_desc = {};
    compute_pipeline_layout_desc.bindGroupLayoutCount = 1;
    compute_pipeline_layout_desc.bindGroupLayouts     = &computeBindGroupLayout;

    WGPUPipelineLayout compute_pipeline_layout = wgpuDeviceCreatePipelineLayout(device,
                                                                                &compute_pipeline_layout_desc);
    assert(compute_pipeline_layout != NULL);

    // create compute pipeline
    WGPUComputePipelineDescriptor computePipelineDesc = {};
    computePipelineDesc.layout = compute_pipeline_layout;
    computePipelineDesc.compute.module = computeMod;
    computePipelineDesc.compute.entryPoint = "main";
    computePipelineDesc.compute.constantCount = 0;
    computePipelineDesc.compute.constants = nullptr;
    computePipeline = wgpuDeviceCreateComputePipeline(device,
                                                      &computePipelineDesc);
    assert(computePipeline != NULL);

    wgpuPipelineLayoutRelease(compute_pipeline_layout);
    wgpuShaderModuleRelease(computeMod);    
}

static void createRenderPipeline() {

    // compile shaders
    WGPUShaderModule vertMod = createShader(triangle_vert_wgsl);
    WGPUShaderModule fragMod = createShader(triangle_frag_wgsl);

    // describe buffer layouts
    WGPUVertexAttribute vertAttrs_0 = {};
    vertAttrs_0.format = WGPUVertexFormat_Float32x2;
    vertAttrs_0.offset = 0;
    vertAttrs_0.shaderLocation = 0;
    WGPUVertexAttribute vertAttrs_1 = {};
    vertAttrs_1.format = WGPUVertexFormat_Float32x2;
    vertAttrs_1.offset = 0;
    vertAttrs_1.shaderLocation = 1;    
    WGPUVertexBufferLayout vertexBufferLayout[2] = {};
    vertexBufferLayout[0].arrayStride    = 2 * sizeof(float);
    vertexBufferLayout[0].stepMode       = WGPUVertexStepMode_Instance,
    vertexBufferLayout[0].attributeCount = 1;
    vertexBufferLayout[0].attributes     = &vertAttrs_0;
    vertexBufferLayout[1].arrayStride    = 2 * sizeof(float);
    vertexBufferLayout[1].stepMode       = WGPUVertexStepMode_Vertex,
    vertexBufferLayout[1].attributeCount = 1;
    vertexBufferLayout[1].attributes     = &vertAttrs_1;

    WGPUColorTargetState colorTarget = {};
    colorTarget.format = swapPref;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment = {};
    fragment.module = fragMod;
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    WGPURenderPipelineDescriptor desc = {};
    desc.fragment = &fragment;

    // Other state
    desc.depthStencil = nullptr;

    desc.vertex.module = vertMod;
    desc.vertex.entryPoint = "main";
    desc.vertex.bufferCount = 2;//0;
    desc.vertex.buffers = vertexBufferLayout;

    desc.multisample.count = 1;
    desc.multisample.mask = 0xFFFFFFFF;
    desc.multisample.alphaToCoverageEnabled = false;

    desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    WGPUBindGroupLayoutEntry groupLayoutEntries[1] = {};
    groupLayoutEntries[0].binding = 0;
    groupLayoutEntries[0].visibility = WGPUShaderStage::WGPUShaderStage_Vertex | WGPUShaderStage::WGPUShaderStage_Fragment;
    groupLayoutEntries[0].buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
    groupLayoutEntries[0].buffer.hasDynamicOffset = false;
    groupLayoutEntries[0].buffer.minBindingSize = 8;

    WGPUBindGroupLayoutDescriptor groupLayoutDesc = {};
    groupLayoutDesc.entryCount = 1;
    groupLayoutDesc.entries = groupLayoutEntries;

    renderBindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &groupLayoutDesc);

    // create render pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &renderBindGroupLayout;

    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

    // create render pipeline
    desc.layout = pipelineLayout;
    renderPipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    wgpuPipelineLayoutRelease(pipelineLayout);
    wgpuShaderModuleRelease(fragMod);
    wgpuShaderModuleRelease(vertMod);    
}

static void createBuffersAndBindGroups() {

    // memory buffer for all particles data of type [(posx,posy),...]
    float particlesData[NUM_PARTICLES * 2];
    memset(particlesData, 0.f, sizeof(particlesData));
    for (uint32_t i = 0; i < NUM_PARTICLES; i++) {
        const size_t chunk       = i * 2;
        particlesData[chunk + 0] = -0.9f + (i * 0.02f);         /* posx */
        particlesData[chunk + 1] = -0.9f;                       /* posy */
    }    

    // Create 2 gpu buffers for binding to both compute and
    // render pipelines
    for (uint32_t i = 0; i < 2; ++i) {
        particlesBuf[i] = createBuffer(particlesData,
                                       sizeof(particlesData),
                                       static_cast<WGPUBufferUsage>(WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage));
    }

    // Create two bind groups for compute pipeline,
    // one for each buffer as the src where the alternate
    // buffer is used as the dst
    for (uint32_t i = 0; i < 2; ++i) {
        WGPUBindGroupEntry bg_entries[2] = {};
        bg_entries[0].binding = 0;
        bg_entries[0].buffer  = particlesBuf[i];
        bg_entries[0].offset  = 0;
        bg_entries[0].size    = sizeof(particlesData);

        bg_entries[1].binding = 1;
        bg_entries[1].buffer  = particlesBuf[(i + 1) % 2];
        bg_entries[1].offset  = 0;
        bg_entries[1].size    = sizeof(particlesData);

        WGPUBindGroupDescriptor bg_desc = {};
        bg_desc.layout        = computeBindGroupLayout;
        bg_desc.entryCount    = 2;
        bg_desc.entries       = bg_entries;

        particleComputeBindGroups[i] = wgpuDeviceCreateBindGroup(device, &bg_desc);
    }

    shapeBuf = createBuffer(shapeData, sizeof(shapeData), WGPUBufferUsage_Vertex);
    uniformBuf = createBuffer(uniformData, sizeof(uniformData), WGPUBufferUsage_Uniform);
}

static void createPipelinesAndBuffers() {

    createComputePipeline();
    createRenderPipeline();
    createBuffersAndBindGroups();
}

static bool prepare_to_draw()
{
    device = emscripten_webgpu_get_device();
    swapPref = WGPUTextureFormat_BGRA8Unorm;

    assert(device);
    // printf("--SP %s:%d %s Device Creation successful\n", __FILE__, __LINE__, __FUNCTION__);

    queue = wgpuDeviceGetQueue(device);

    // printf("--SP %s:%d %s Getting Device Queue successful\n", __FILE__, __LINE__, __FUNCTION__);
    // Create swap chain
    WGPUSwapChainDescriptor swapDesc = {};
    WGPUSurface surface = {};

	WGPUSurfaceDescriptorFromCanvasHTMLSelector canvDesc = {};
	canvDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
	canvDesc.selector = "!canvas";
	
	WGPUSurfaceDescriptor surfDesc = {};
	surfDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&canvDesc);
	
	surface = wgpuInstanceCreateSurface(nullptr, &surfDesc);
	
	swapDesc.usage  = WGPUTextureUsage_RenderAttachment;
	swapDesc.format = WGPUTextureFormat_BGRA8Unorm;
	swapDesc.width  = 1024;
	swapDesc.height = 768;
	swapDesc.presentMode = WGPUPresentMode_Fifo;
	swapchain = wgpuDeviceCreateSwapChain(device, surface, &swapDesc);

    // printf("--SP %s:%d %s Creating Swap chain successful\n", __FILE__, __LINE__, __FUNCTION__);

    createPipelinesAndBuffers();

    // printf("--SP %s:%d %s Creating pipeline and buffers successful\n", __FILE__, __LINE__, __FUNCTION__);

    return true;
}

static bool draw() {

    static int frame_index = 0;
    frame_index++;

    // printf("draw index = %d\n", frame_index);

    WGPUTextureView backBufView = wgpuSwapChainGetCurrentTextureView(swapchain);			// create textureView

    WGPURenderPassColorAttachment colorDesc = {};
    colorDesc.view = backBufView;
    colorDesc.loadOp = WGPULoadOp_Clear;
    colorDesc.storeOp = WGPUStoreOp_Store;
    colorDesc.clearValue.r = 0.0f;
    colorDesc.clearValue.g = 1.0f;
    colorDesc.clearValue.b = 0.0f;
    colorDesc.clearValue.a = 1.0f;

    WGPURenderPassDescriptor renderPass = {};
    renderPass.colorAttachmentCount = 1;
    renderPass.colorAttachments = &colorDesc;

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);			// create encoder

    // =============
    // compute pass
    // =============
    WGPUComputePassEncoder cpass = wgpuCommandEncoderBeginComputePass(encoder, NULL);
    wgpuComputePassEncoderSetPipeline(cpass, computePipeline);
    wgpuComputePassEncoderSetBindGroup(cpass,
                                       0,
                                       particleComputeBindGroups[frame_index % 2],
                                       0,
                                       NULL);
    uint32_t work_group_count = ceil((float)NUM_PARTICLES / WORKGROUP_SIZE);
    wgpuComputePassEncoderDispatchWorkgroups(cpass, work_group_count, 1, 1);
    wgpuComputePassEncoderEnd(cpass);
    wgpuComputePassEncoderRelease(cpass);
    cpass = nullptr;

    // =============
    // render pass    
    // =============
    WGPURenderPassEncoder rpass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPass);	// create pass

    wgpuRenderPassEncoderSetPipeline(rpass, renderPipeline);

    wgpuRenderPassEncoderSetVertexBuffer(rpass, 0, particlesBuf[frame_index % 2], 0, NUM_PARTICLES * (sizeof(float)*2));
    wgpuRenderPassEncoderSetVertexBuffer(rpass, 1, shapeBuf, 0, sizeof(shapeData));

    WGPUBindGroupEntry uniformBindGroupEntries[1] = {};
    uniformBindGroupEntries[0].binding = 0;
    uniformBindGroupEntries[0].buffer = uniformBuf;
    uniformBindGroupEntries[0].offset = 0;
    uniformBindGroupEntries[0].size = 8;
    WGPUBindGroupDescriptor uniformBindGroupDesc = {};
    uniformBindGroupDesc.layout = renderBindGroupLayout;
    uniformBindGroupDesc.entryCount = 1;
    uniformBindGroupDesc.entries = uniformBindGroupEntries;
    WGPUBindGroup uniformBindGroup = wgpuDeviceCreateBindGroup(device, &uniformBindGroupDesc);

    wgpuRenderPassEncoderSetBindGroup(rpass, 0, uniformBindGroup, 0, nullptr);

    wgpuRenderPassEncoderDraw(rpass, (sizeof(shapeData)/(sizeof(float)*2)), NUM_PARTICLES, 0, 0);

    wgpuBindGroupRelease(uniformBindGroup);
    uniformBindGroup = nullptr;

    wgpuRenderPassEncoderEnd(rpass);
    wgpuRenderPassEncoderRelease(rpass);												    // release pass
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);				// create commands
    wgpuCommandEncoderRelease(encoder);														// release encoder

    wgpuQueueSubmit(queue, 1, &commands);
    wgpuCommandBufferRelease(commands);														// release commands
    // wgpuSwapChainPresent(swapchain);
    wgpuTextureViewRelease(backBufView);													// release textureView

    jsTransferImageWebGPU();

    return true;
}

extern "C" {

    EMSCRIPTEN_KEEPALIVE void render_on_offscreen_canvas()
    {
        static bool isPrepared = false;
        if (!isPrepared)
        {
            prepare_to_draw();
            isPrepared = true;
        }

        while (1)
        {
            printf("C++Land: render_on_offscreen_canvas() \n");
            draw();
        }
    }
}