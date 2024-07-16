// TODO: Part 1b
#include "FSLogo.h"
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
	#pragma comment(lib, "shaderc_combined.lib") 
#endif

#define PI 3.14f

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

class Renderer
{
	// TODO: Part 2b
	// TODO: Part 3a
	
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VkRenderPass renderPass;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;

	// TODO: Part 1g
	VkBuffer indexHandle = nullptr;
	VkDeviceMemory indexData = nullptr;

	// TODO: Part 2c
	VkShaderModule vertexShader = nullptr;
	VkShaderModule fragmentShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	// TODO: Part 2d
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkBuffer> uniformHandles;
	std::vector<VkDeviceMemory> uniformDatas;

	// TODO: Part 3d
	std::vector<VkBuffer> storageHandles;
	std::vector<VkDeviceMemory> storageDatas;

	unsigned int windowWidth, windowHeight;

	struct VEC3
	{
		float x, y, z;
	};

	struct VERT3
	{
		VEC3 pos, uvw, nrm;
	};

	// TODO: Part 2a
	GW::MATH::GMatrix matrixMath;
	GW::MATH::GMATRIXF cameraMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF cameraInvertedMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF projectionMatrix = GW::MATH::GIdentityMatrixF;

	GW::MATH::GVECTORF translateCamera = { 0.75f, 0.25f, -1.5f, 0 }; //can the z be more than 1?
	GW::MATH::GVECTORF translateLight = { 1.0f, 1.0f, 1.0f, 0 };
	GW::MATH::GVECTORF origin = { 0.15f, 0.75f, 0, 1 };
	GW::MATH::GVECTORF upVec = { 0, 1, 0, 0 };
	float aspectRatio = 0.0f;

	GW::MATH::GMATRIXF lightRotX = GW::MATH::GIdentityMatrixF;
	GW::MATH::GVECTORF lightDir = { -1, -1, 2, 0 }; //normalize this?
	GW::MATH::GVECTORF lightColor = { 0.9f, 0.9f, 1, 1 };

	// TODO: Part 2b // TODO: Part 4d
	struct SHADER_SCENE_DATA
	{
		GW::MATH::GVECTORF lightDirection, lightColor;
		GW::MATH::GMATRIXF viewMatrix, projectionMatrix;
	};
	SHADER_SCENE_DATA sceneData;

	unsigned swapChainCount;
	// TODO: Part 3a
	struct INSTANCE_DATA
	{
		GW::MATH::GMATRIXF worldMatrix;
		OBJ_ATTRIBUTES material;
	};
	std::vector<INSTANCE_DATA> perFrame;


public:

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		UpdateWindowDimensions();

		// TODO: Part 2a
		matrixMath.Create();
		vlk.GetAspectRatio(aspectRatio);
		vlk.GetSwapchainImageCount(swapChainCount);
		descriptorSets.resize(swapChainCount);
		uniformDatas.resize(swapChainCount);
		uniformHandles.resize(swapChainCount);
		storageHandles.resize(swapChainCount);
		storageDatas.resize(swapChainCount);
		perFrame.resize(swapChainCount);
		createViewMatrix();
		createProjectionMatrix(65);
		createRotTranMatrix(65);

		// TODO: Part 2b // TODO: Part 4d
		//SHADER_SCENE_DATA sceneData;
		/*sceneData.lightColor = lightColor;
		sceneData.lightDirection = lightDir;
		sceneData.viewMatrix = cameraInvertedMatrix;
		sceneData.projectionMatrix = projectionMatrix;*/

		// TODO: part 3a
		for (int i = 0; i < perFrame.size(); ++i) {
			perFrame[i].worldMatrix = GW::MATH::GIdentityMatrixF;
			perFrame[i].material = FSLogo_materials[i].attrib;
		}

		InitializeGraphics();
		BindShutdownCallback();
	}

private:
	void UpdateWindowDimensions()
	{
		win.GetClientWidth(windowWidth);
		win.GetClientHeight(windowHeight);
	}

	void InitializeGraphics()
	{
		GetHandlesFromSurface();
		InitializeVertexBuffer();
		// TODO: Part 1g
		InitializeIndexBuffer();
		// TODO: Part 2d // TODO: Part 3d
		InitializeUniformBuffer();
		InitializeStorageBuffer();
		CompileShaders();
		InitializeDescriptorSets();
		InitializeGraphicsPipeline();
	}

	void GetHandlesFromSurface()
	{
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);
		vlk.GetRenderPass((void**)&renderPass);
	}

	void InitializeVertexBuffer()
	{
		// TODO: Part 1c
		/*float verts[] = 
		{
			0,   0.5f,
			0.5f, -0.5f,
			-0.5f, -0.5f
		};*/

		CreateVertexBuffer(&FSLogo_vertices[0], sizeof(FSLogo_vertices));
	}

	void InitializeIndexBuffer()
	{
		// TODO: Part 1g
		CreateIndexBuffer(&FSLogo_indices[0], sizeof(FSLogo_indices));
	}

	void InitializeUniformBuffer()
	{
		CreateUniformBuffer(&sceneData, sizeof(SHADER_SCENE_DATA));
	}

	void InitializeStorageBuffer()
	{
		CreateStorageBuffer(perFrame.data(), sizeof(INSTANCE_DATA));
	}

	float DegreesToRadians(float degrees) {
		return (degrees * (PI / 180.0f));
	}

	void createRotTranMatrix(float degrees) {
		float rad = DegreesToRadians(degrees);

		matrixMath.TranslateGlobalF(lightRotX, translateLight, lightRotX);
		//matrixMath.RotateXGlobalF(lightRotX, rad, lightRotX);
		matrixMath.LookAtLHF(lightRotX.row4, lightDir, upVec, lightRotX); //will this work?
	}

	void createViewMatrix() {
		matrixMath.TranslateGlobalF(cameraMatrix, translateCamera, cameraMatrix);
		matrixMath.LookAtLHF(cameraMatrix.row4, origin, upVec, cameraInvertedMatrix);
	}

	void createProjectionMatrix(float degrees) {
		float rad = DegreesToRadians(degrees);

		matrixMath.ProjectionVulkanLHF(rad, aspectRatio, 0.1f, 100, projectionMatrix);
	}

	void CreateVertexBuffer(const void* data, unsigned int sizeInBytes)
	{
		GvkHelper::create_buffer(physicalDevice, device, sizeInBytes,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		GvkHelper::write_to_buffer(device, vertexData, data, sizeInBytes);
	}

	void CreateIndexBuffer(const void* data, unsigned int sizeInBytes)
	{
		GvkHelper::create_buffer(physicalDevice, device, sizeInBytes,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexHandle, &indexData);
		GvkHelper::write_to_buffer(device, indexData, data, sizeInBytes);
	}

	void CreateUniformBuffer(const void* _data, unsigned int sizeInBytes)
	{
		for (unsigned i = 0; i < uniformHandles.size(); i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeInBytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&uniformHandles[i], &uniformDatas[i]);
			GvkHelper::write_to_buffer(device, uniformDatas[i], _data, sizeInBytes);
		}
	}

	void CreateStorageBuffer(const void* _data, unsigned int sizeInBytes)
	{
		for (unsigned i = 0; i < storageHandles.size(); i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&storageHandles[i], &storageDatas[i]);
			GvkHelper::write_to_buffer(device, storageDatas[i], _data, sizeInBytes);
		}
	}

	void CompileShaders()
	{
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = CreateCompileOptions();

		CompileVertexShader(compiler, options);
		CompilePixelShader(compiler, options);

		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}

	shaderc_compile_options_t CreateCompileOptions()
	{
		shaderc_compile_options_t retval = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(retval, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(retval, false); // TODO: Part 2g
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(retval);
#endif
		return retval;	
	}

	void CompileVertexShader(const shaderc_compiler_t& compiler, const shaderc_compile_options_t& options)
	{
		std::string vertexShaderSource = ReadFileIntoString("../VertexShader.hlsl");

		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, vertexShaderSource.c_str(), vertexShaderSource.length(),
			shaderc_vertex_shader, "main.vert", "main", options);
		
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", shaderc_result_get_error_message(result));
			abort();
			return;
		}
		
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader);
		
		shaderc_result_release(result); // done
	}
	
	void CompilePixelShader(const shaderc_compiler_t& compiler, const shaderc_compile_options_t& options)
	{
		std::string fragmentShaderSource = ReadFileIntoString("../FragmentShader.hlsl");
		
		shaderc_compilation_result_t result;
		
		result = shaderc_compile_into_spv( // compile
			compiler, fragmentShaderSource.c_str(), fragmentShaderSource.length(),
			shaderc_fragment_shader, "main.frag", "main", options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
		{
			PrintLabeledDebugString("Fragment Shader Errors:\n", shaderc_result_get_error_message(result));
			abort();
			return;
		}
		
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &fragmentShader);
		
		shaderc_result_release(result); // done
	}

	void CreateDescriptorSetLayout() {

		VkDescriptorSetLayoutBinding layoutBinding[2];
		{
			layoutBinding[0].binding = 0; // Binding number in the shader
			layoutBinding[0].descriptorCount = 1;
			layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinding[0].pImmutableSamplers = nullptr; // For sampled images

			layoutBinding[1].binding = 1; // Binding number in the shader
			layoutBinding[1].descriptorCount = 1;
			layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinding[1].pImmutableSamplers = nullptr; // For sampled images
		};
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.flags = 0; //maybe?
		layoutInfo.bindingCount = ARRAYSIZE(layoutBinding);
		layoutInfo.pBindings = layoutBinding;
		layoutInfo.pNext = nullptr; //maybe?

		auto result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
		int h = 90;

	}

	void CreateDescriptorPool() {

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		std::vector<VkDescriptorPoolSize> poolSize;
		poolSize.resize(swapChainCount);
		poolSize[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChainCount };
		poolSize[1] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, swapChainCount };
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = swapChainCount;
		poolInfo.flags = 0;
		poolInfo.pNext = nullptr;
		vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
	}

	void AllocateDescriptorSet() {

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pNext = nullptr;

		for (auto& i : descriptorSets) {
			auto result = vkAllocateDescriptorSets(device, &allocInfo, &i);
			int k = 69;
		}
	}

	void LinkDescriptorSet2Buffer() {

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.descriptorCount = 1; // Number of descriptors to update
		descriptorWrite.dstArrayElement = 0; // First array element to update
		descriptorWrite.dstBinding = 0; // The binding number in the shader
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Type of the descriptor
		for (int i = 0; i < swapChainCount; ++i) {
			descriptorWrite.dstSet = descriptorSets[i];
			VkDescriptorBufferInfo dbinfo = { uniformHandles[i], 0, VK_WHOLE_SIZE };
			descriptorWrite.pBufferInfo = &dbinfo;
			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}

	}

	void InitializeDescriptorSets() {

		CreateDescriptorSetLayout();
		CreateDescriptorPool();
		AllocateDescriptorSet();
		LinkDescriptorSet2Buffer();

	}

	void InitializeGraphicsPipeline()
	{
		// Create Pipeline & Layout (Thanks Tiny!)
		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info[0].module = vertexShader;
		stage_create_info[0].pName = "main";

		// Create Stage Info for Fragment Shader
		stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info[1].module = fragmentShader;
		stage_create_info[1].pName = "main";

		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = CreateVkPipelineInputAssemblyStateCreateInfo();
		VkVertexInputBindingDescription vertex_binding_description = CreateVkVertexInputBindingDescription();

		VkVertexInputAttributeDescription vertex_attribute_description[3];
		vertex_attribute_description[0].binding = 0;
		vertex_attribute_description[0].location = 0;
		vertex_attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[0].offset = 0;

		vertex_attribute_description[1].binding = 0;
		vertex_attribute_description[1].location = 1;
		vertex_attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[1].offset = 12;

		vertex_attribute_description[2].binding = 0;
		vertex_attribute_description[2].location = 2;
		vertex_attribute_description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertex_attribute_description[2].offset = 24;

		VkPipelineVertexInputStateCreateInfo input_vertex_info = CreateVkPipelineVertexInputStateCreateInfo(&vertex_binding_description, 1, vertex_attribute_description, 3);
		VkViewport viewport = CreateViewportFromWindowDimensions();
		VkRect2D scissor = CreateScissorFromWindowDimensions(); 
		VkPipelineViewportStateCreateInfo viewport_create_info = CreateVkPipelineViewportStateCreateInfo(&viewport, 1, &scissor, 1);
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = CreateVkPipelineRasterizationStateCreateInfo();
		VkPipelineMultisampleStateCreateInfo multisample_create_info = CreateVkPipelineMultisampleStateCreateInfo();
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = CreateVkPipelineDepthStencilStateCreateInfo();
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = CreateVkPipelineColorBlendAttachmentState();
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = CreateVkPipelineColorBlendStateCreateInfo(&color_blend_attachment_state, 1);

		// Dynamic State 
		VkDynamicState dynamic_states[2] = 
		{
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT, 
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_create_info = CreateVkPipelineDynamicStateCreateInfo(dynamic_states, 2);

		CreatePipelineLayout();

		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = stage_create_info;
		pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		pipeline_create_info.pVertexInputState = &input_vertex_info;
		pipeline_create_info.pViewportState = &viewport_create_info;
		pipeline_create_info.pRasterizationState = &rasterization_create_info;
		pipeline_create_info.pMultisampleState = &multisample_create_info;
		pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_create_info;
		pipeline_create_info.layout = pipelineLayout;
		pipeline_create_info.renderPass = renderPass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline);
	}

	VkPipelineInputAssemblyStateCreateInfo CreateVkPipelineInputAssemblyStateCreateInfo()
	{
		VkPipelineInputAssemblyStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		retval.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		retval.primitiveRestartEnable = false;
		return retval;
	}

	VkVertexInputBindingDescription CreateVkVertexInputBindingDescription()
	{
		// TODO: Part 1e
		VkVertexInputBindingDescription retval = {};
		retval.binding = 0;
		retval.stride = sizeof(VERT3);
		retval.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return retval;
	}

	VkPipelineVertexInputStateCreateInfo CreateVkPipelineVertexInputStateCreateInfo(
		VkVertexInputBindingDescription* bindingDescriptions, uint32_t bindingCount,
		VkVertexInputAttributeDescription* attributeDescriptions, uint32_t attributeCount)
	{
		VkPipelineVertexInputStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		retval.vertexBindingDescriptionCount = bindingCount;
		retval.pVertexBindingDescriptions = bindingDescriptions;
		retval.vertexAttributeDescriptionCount = attributeCount;
		retval.pVertexAttributeDescriptions = attributeDescriptions;
		return retval;
	}

	VkViewport CreateViewportFromWindowDimensions()
	{
		VkViewport retval = {};
		retval.x = 0;
		retval.y = 0;
		retval.width = static_cast<float>(windowWidth);
		retval.height = static_cast<float>(windowHeight);
		retval.minDepth = 0;
		retval.maxDepth = 1;
		return retval;
	}

	VkRect2D CreateScissorFromWindowDimensions()
	{
		VkRect2D retval = {};
		retval.offset.x = 0;
		retval.offset.y = 0;
		retval.extent.width = windowWidth;
		retval.extent.height = windowHeight;
		return retval;
	}

	VkPipelineViewportStateCreateInfo CreateVkPipelineViewportStateCreateInfo(VkViewport* viewports, uint32_t viewportCount, VkRect2D* scissors, uint32_t scissorCount)
	{
		VkPipelineViewportStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		retval.viewportCount = viewportCount;
		retval.pViewports = viewports;
		retval.scissorCount = scissorCount;
		retval.pScissors = scissors;
		return retval;
	}

	VkPipelineRasterizationStateCreateInfo CreateVkPipelineRasterizationStateCreateInfo()
	{
		VkPipelineRasterizationStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		retval.rasterizerDiscardEnable = VK_FALSE;
		retval.polygonMode = VK_POLYGON_MODE_FILL;
		retval.lineWidth = 1.0f;
		retval.cullMode = VK_CULL_MODE_BACK_BIT;
		retval.frontFace = VK_FRONT_FACE_CLOCKWISE;
		retval.depthClampEnable = VK_FALSE;
		retval.depthBiasEnable = VK_FALSE;
		retval.depthBiasClamp = 0.0f;
		retval.depthBiasConstantFactor = 0.0f;
		retval.depthBiasSlopeFactor = 0.0f;
		return retval;
	}

	VkPipelineMultisampleStateCreateInfo CreateVkPipelineMultisampleStateCreateInfo()
	{
		VkPipelineMultisampleStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		retval.sampleShadingEnable = VK_FALSE;
		retval.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		retval.minSampleShading = 1.0f;
		retval.pSampleMask = VK_NULL_HANDLE;
		retval.alphaToCoverageEnable = VK_FALSE;
		retval.alphaToOneEnable = VK_FALSE;
		return retval;
	}

	VkPipelineDepthStencilStateCreateInfo CreateVkPipelineDepthStencilStateCreateInfo()
	{
		VkPipelineDepthStencilStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		retval.depthTestEnable = VK_TRUE;
		retval.depthWriteEnable = VK_TRUE;
		retval.depthCompareOp = VK_COMPARE_OP_LESS;
		retval.depthBoundsTestEnable = VK_FALSE;
		retval.minDepthBounds = 0.0f;
		retval.maxDepthBounds = 1.0f;
		retval.stencilTestEnable = VK_FALSE;
		return retval;
	}

	VkPipelineColorBlendAttachmentState CreateVkPipelineColorBlendAttachmentState()
	{
		VkPipelineColorBlendAttachmentState retval = {};
		retval.colorWriteMask = 0xF;
		retval.blendEnable = VK_FALSE;
		retval.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		retval.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		retval.colorBlendOp = VK_BLEND_OP_ADD;
		retval.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		retval.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		retval.alphaBlendOp = VK_BLEND_OP_ADD;
		return retval;
	}

	VkPipelineColorBlendStateCreateInfo CreateVkPipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* attachmentStates, uint32_t attachmentCount)
	{
		VkPipelineColorBlendStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		retval.logicOpEnable = VK_FALSE;
		retval.logicOp = VK_LOGIC_OP_COPY;
		retval.attachmentCount = attachmentCount;
		retval.pAttachments = attachmentStates;
		retval.blendConstants[0] = 0.0f;
		retval.blendConstants[1] = 0.0f;
		retval.blendConstants[2] = 0.0f;
		retval.blendConstants[3] = 0.0f;
		return retval;
	}

	VkPipelineDynamicStateCreateInfo CreateVkPipelineDynamicStateCreateInfo(VkDynamicState* dynamicStates, uint32_t dynamicStateCount)
	{
		VkPipelineDynamicStateCreateInfo retval = {};
		retval.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		retval.dynamicStateCount = dynamicStateCount;
		retval.pDynamicStates = dynamicStates;
		return retval;
	}

	void CreatePipelineLayout()
	{
		// Descriptor pipeline layout
		// TODO: Part 2d // TODO: Part 3d
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &descriptorSetLayout;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;

		vkCreatePipelineLayout(device, &pipeline_layout_create_info,nullptr, &pipelineLayout);
	}

	void BindShutdownCallback()
	{
		// GVulkanSurface will inform us when to release any allocated resources
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
			});
	}


public:

	void Update()
	{
		// Adjust CPU data to reflect what we want to draw
		/*for (int i = 0; i < 240; ++i) {
			GW::MATH::GMatrix::RotateZLocalF(instanceData.instance_transforms[i],
				0.0001f, instanceData.instance_transforms[i]);
		}*/
		// Copy data to this frame's buffer
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		GvkHelper::write_to_buffer(device,
			uniformDatas[currentBuffer], &sceneData, sizeof(sceneData));
		GvkHelper::write_to_buffer(device,
			storageDatas[currentBuffer], perFrame.data(), sizeof(INSTANCE_DATA));
	}

	void Render()
	{
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer commandBuffer;
		vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);

		// TODO: Part 3i
		// TODO: Part 2a
		sceneData.lightColor = lightColor;
		sceneData.lightDirection = lightDir;
		sceneData.viewMatrix = cameraInvertedMatrix;
		sceneData.projectionMatrix = projectionMatrix;

		//VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();
		SetUpPipeline(commandBuffer);
		// TODO: Part 3i
		// TODO: Part 1h
		vkCmdBindIndexBuffer(commandBuffer, indexHandle, 0, VK_INDEX_TYPE_UINT32);

		// TODO: Part 2e
		/*GvkHelper::write_to_buffer(device, uniformDatas[0], &sceneData, sizeof(sceneData));
		GvkHelper::write_to_buffer(device, storageDatas[0], perFrame.data(), sizeof(INSTANCE_DATA));*/
		Update();
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &descriptorSets[currentBuffer], 0, NULL);

		// TODO: Part 3f
		vkCmdDrawIndexed(commandBuffer, ARRAYSIZE(FSLogo_indices), 1, 0, 0, 0); // TODO: Part 1d
	}

private:

	VkCommandBuffer GetCurrentCommandBuffer()
	{
		VkCommandBuffer retval;
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		vlk.GetCommandBuffer(currentBuffer, (void**)&retval);
		return retval;
	}

	void SetUpPipeline(VkCommandBuffer& commandBuffer)
	{
		UpdateWindowDimensions(); // what is the current client area dimensions?
		SetViewport(commandBuffer);
		SetScissor(commandBuffer);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		BindVertexBuffers(commandBuffer);
	}

	void SetViewport(const VkCommandBuffer& commandBuffer)
	{
		VkViewport viewport = CreateViewportFromWindowDimensions();
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void SetScissor(const VkCommandBuffer& commandBuffer)
	{
		VkRect2D scissor = CreateScissorFromWindowDimensions();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void BindVertexBuffers(VkCommandBuffer& commandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
	}


	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		// TODO: Part 1g
		vkDestroyBuffer(device, indexHandle, nullptr);
		vkFreeMemory(device, indexData, nullptr);

		// TODO: Part 2d
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		for (auto& i : uniformHandles) {
			vkDestroyBuffer(device, i, nullptr);
		}
		for (auto& i : uniformDatas) {
			vkFreeMemory(device, i, nullptr);
		}

		// TODO: Part 3d
		for (auto& i : storageHandles) {
			vkDestroyBuffer(device, i, nullptr);
		}
		for (auto& i : storageDatas) {
			vkFreeMemory(device, i, nullptr);
		}

		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, fragmentShader, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};
