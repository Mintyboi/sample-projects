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
WGPURenderPipeline pipeline;
WGPUBuffer vertBuf; // vertex buffer with triangle position and colours
WGPUBuffer indxBuf; // index buffer
WGPUBuffer uniformBuf; //uniform buffer

static char const triangle_vert_wgsl[] = R"(
	struct VertexIn {
		@location(0) aPos : vec2<f32>,
		@location(1) aCol : vec3<f32>
	};
	struct VertexOut {
		@location(0) vCol : vec3<f32>,
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
        var xScale: f32 = param.scale1;
        var yScale: f32 = param.scale2;
		var output : VertexOut;
		output.Position = vec4<f32>(vec3<f32>(input.aPos[0]/xScale, input.aPos[1]/yScale, 1.0), 1.0);
		output.vCol = input.aCol;
		return output;
	}
)";


static char const triangle_frag_wgsl[] = R"(
	@fragment
	fn main(@location(0) vCol : vec3<f32>) -> @location(0) vec4<f32> {
		return vec4<f32>(vCol, 1.0);
	}
)";


// create the buffers (x, y, r, g, b)
float const vertData[] = {
    -0.8f, -0.8f, 0.0f, 0.0f, 1.0f, // BL
    0.8f, -0.8f, 0.0f, 1.0f, 0.0f, // BR
    -0.0f,  0.8f, 1.0f, 0.0f, 0.0f, // top
};
uint16_t const indxData[] = {
    0, 1, 2,
    0 // padding (better way of doing this?)
};

float const uniformData[] = {
    1.0,
    1.0
};

WGPUBindGroupLayout groupLayouts[1] = {};

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

/**
 * Bare minimum pipeline to draw a triangle using the above shaders.
 */
static void createPipelineAndBuffers() {
    // compile shaders
    WGPUShaderModule vertMod = createShader(triangle_vert_wgsl);
    WGPUShaderModule fragMod = createShader(triangle_frag_wgsl);

    // describe buffer layouts
    WGPUVertexAttribute vertAttrs[2] = {};
    vertAttrs[0].format = WGPUVertexFormat_Float32x2;
    vertAttrs[0].offset = 0;
    vertAttrs[0].shaderLocation = 0;
    vertAttrs[1].format = WGPUVertexFormat_Float32x3;
    vertAttrs[1].offset = 2 * sizeof(float);
    vertAttrs[1].shaderLocation = 1;
    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.arrayStride = 5 * sizeof(float);
    vertexBufferLayout.attributeCount = 2;
    vertexBufferLayout.attributes = vertAttrs;

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
    desc.vertex.bufferCount = 1;//0;
    desc.vertex.buffers = &vertexBufferLayout;

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

    groupLayouts[0] = wgpuDeviceCreateBindGroupLayout(device, &groupLayoutDesc);

    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = groupLayouts;

    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &pipelineLayoutDesc);

    desc.layout = pipelineLayout;
    pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    wgpuShaderModuleRelease(fragMod);
    wgpuShaderModuleRelease(vertMod);

    vertBuf = createBuffer(vertData, sizeof(vertData), WGPUBufferUsage_Vertex);
    indxBuf = createBuffer(indxData, sizeof(indxData), WGPUBufferUsage_Index);

    uniformBuf = createBuffer(uniformData, sizeof(uniformData), WGPUBufferUsage_Uniform);
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

    createPipelineAndBuffers();

    // printf("--SP %s:%d %s Creating pipeline and buffers successful\n", __FILE__, __LINE__, __FUNCTION__);

    return true;
}

static bool draw() {

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
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPass);	// create pass

    // draw the triangle (comment these five lines to simply clear the screen)
    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, vertBuf, 0, sizeof(vertData));
    wgpuRenderPassEncoderSetIndexBuffer(pass, indxBuf, WGPUIndexFormat_Uint16, 0, sizeof(indxData));

    WGPUBindGroupEntry uniformBindGroupEntries[1] = {};
    uniformBindGroupEntries[0].binding = 0;
    uniformBindGroupEntries[0].buffer = uniformBuf;
    uniformBindGroupEntries[0].offset = 0;
    uniformBindGroupEntries[0].size = 8;
    WGPUBindGroupDescriptor uniformBindGroupDesc = {};
    uniformBindGroupDesc.layout = groupLayouts[0];
    uniformBindGroupDesc.entryCount = 1;
    uniformBindGroupDesc.entries = uniformBindGroupEntries;
    WGPUBindGroup uniformBindGroup = wgpuDeviceCreateBindGroup(device, &uniformBindGroupDesc);

    wgpuRenderPassEncoderSetBindGroup(pass, 0, uniformBindGroup, 0, nullptr);

    wgpuRenderPassEncoderDrawIndexed(pass, 3, 1, 0, 0, 0);

    wgpuBindGroupRelease(uniformBindGroup);
    uniformBindGroup = nullptr;

    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);														// release pass
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);				// create commands
    wgpuCommandEncoderRelease(encoder);														// release encoder

    wgpuQueueSubmit(queue, 1, &commands);
    wgpuCommandBufferRelease(commands);														// release commands
    wgpuSwapChainPresent(swapchain);
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