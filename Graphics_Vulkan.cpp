/*===================================================================================
*	Date : 2022/10/21(Fri)
*		Author	: Gakuto.S
*		File	: Graphics_Vulkan.cpp
*		Detail	:
===================================================================================*/
#include <fstream>

#include "Graphics_Vulkan.h"
using namespace structure;

/* Initialize */
bool GraphicsVulkan::Init(int width, int height, void* handle)
{
	if (!this->CreateInstance())
		return false;

	if (!this->SelectPhysicalDevice())
		return false;

	if (!this->CreateDevice())
		return false;

	if (!this->CreateCommandPool())
		return false;

	if (!this->CreateSurface((HWND)handle))
		return false;

	if (!this->SelectSurfaceFormat())
		return false;

	if (!this->CreateSwapchain(width, height))
		return false;

	if (!this->CreateDepthBuffer())
		return false;

	if (!this->CreateRenderPass())
		return false;

	if (!this->CreateFrameBuffer())
		return false;

	if (!this->CreateFence())
		return false;

	if (!this->CreateSemaphores())
		return false;

	if (!this->CreateConstantBuffer())
		return false;

	if (!this->CreatePipeline())
		return false;

	return true;
}

/* Uninitialize */
void GraphicsVulkan::Uninit()
{
	vkDeviceWaitIdle(m_device);

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_pipeline, nullptr);

	for (auto& v : m_vertexBuffers)
	{
		vkFreeMemory(m_device, v.memory, nullptr);
		vkDestroyBuffer(m_device, v.buffer, nullptr);
	}

	for (auto& i : m_indexBuffers)
	{
		vkFreeMemory(m_device, i.memory, nullptr);
		vkDestroyBuffer(m_device, i.buffer, nullptr);
	}
		
	
	vkFreeCommandBuffers(m_device, m_commandPool, uint32_t(m_commands.size()), m_commands.data());
	m_commands.clear();

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	
	for (auto& v : m_frameBuffers)
		vkDestroyFramebuffer(m_device, v, nullptr);
	m_frameBuffers.clear();

	vkFreeMemory(m_device, m_depthBufferMemory, nullptr);
	vkDestroyImage(m_device, m_depthBuffer, nullptr);
	vkDestroyImageView(m_device, m_depthBufferView, nullptr);

	for (auto& v : m_swapchainViews)
		vkDestroyImageView(m_device, v, nullptr);
	m_swapchainViews.clear();
	
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	for (auto& f : m_fences)
		vkDestroyFence(m_device, f, nullptr);
	m_fences.clear();

	vkDestroySemaphore(m_device, m_renderSem, nullptr);
	vkDestroySemaphore(m_device, m_presentSem, nullptr);

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

/* Clear screen */
void GraphicsVulkan::Clear()
{
	vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_presentSem, VK_NULL_HANDLE, &m_imageIndex);
	vkWaitForFences(m_device, 1, &m_fences[m_imageIndex], VK_TRUE, UINT64_MAX);

	std::array<VkClearValue, 2> clearValue
	{
		{{0.0f, 0.5f, 0.0f, 1.0f},	// color
		{1.0f, 0}}					// depth
	};

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass			= m_renderPass;
	renderPassBeginInfo.framebuffer			= m_frameBuffers[m_imageIndex];
	renderPassBeginInfo.renderArea.offset	= { 0, 0 };
	renderPassBeginInfo.renderArea.extent	= m_swapchainExtent;
	renderPassBeginInfo.pClearValues		= clearValue.data();
	renderPassBeginInfo.clearValueCount		= uint32_t(clearValue.size());

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(m_commands[m_imageIndex], &beginInfo);
	vkCmdBeginRenderPass(m_commands[m_imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

/* Present buffer */
void GraphicsVulkan::Present()
{
	vkCmdEndRenderPass(m_commands[m_imageIndex]);
	vkEndCommandBuffer(m_commands[m_imageIndex]);

	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &m_commands[m_imageIndex];
	submitInfo.pWaitDstStageMask	= &waitStageMask;
	submitInfo.waitSemaphoreCount	= 1;
	submitInfo.pWaitSemaphores		= &m_presentSem;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores	= &m_renderSem;
	vkResetFences(m_device, 1, &m_fences[m_imageIndex]);
	vkQueueSubmit(m_deviceQueue, 1, &submitInfo, m_fences[m_imageIndex]);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= &m_swapchain;
	presentInfo.pImageIndices		= &m_imageIndex;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= &m_renderSem;
	vkQueuePresentKHR(m_deviceQueue, &presentInfo);
}

/* Create vertex buffer and index buffer */
int GraphicsVulkan::CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataNum, const unsigned int* iData, size_t iDataNum)
{
	int retIndex = m_vertexBuffers.size();

	BufferObject vertex = this->CreateBuffer(sizeof(Vertex3D) * vDataNum, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	BufferObject index  = this->CreateBuffer(sizeof(unsigned int) * iDataNum, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	VkResult ret{};
	{// Write data of vertices to memory
		void* map;
		ret = vkMapMemory(m_device, vertex.memory, 0, VK_WHOLE_SIZE, 0, &map);
		if (ret != VK_SUCCESS)
			return -1;

		memcpy(map, vData, sizeof(Vertex3D) * vDataNum);
		vkUnmapMemory(m_device, vertex.memory);
	}

	{// Write data of indices to memory
		void* map;
		ret = vkMapMemory(m_device, index.memory, 0, VK_WHOLE_SIZE, 0, &map);
		if (ret != VK_SUCCESS)
			return -1;

		memcpy(map, iData, sizeof(unsigned int) * iDataNum);
		vkUnmapMemory(m_device, index.memory);
	}

	m_vertexBuffers.push_back(vertex);
	m_indexBuffers.push_back(index);
	m_indexCounts.push_back(iDataNum);

	return retIndex;
}

void GraphicsVulkan::SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos3, const DirectX::XMFLOAT3 rot3, const DirectX::XMFLOAT3 scl3)
{
	DirectX::XMMATRIX trl, rot, scl;
	trl = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos3));
	rot = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rot3));
	scl = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scl3));

	m_ShaderParams.world = //DirectX::XMMatrixTranspose(scl * rot * trl);
	m_ShaderParams.world = scl * rot * trl;
}

void GraphicsVulkan::SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up)
{
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat3(&pos), DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
	m_ShaderParams.view = view; //DirectX::XMMatrixTranspose(view);
}

void GraphicsVulkan::SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ)
{
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, nearZ, farZ);
	m_ShaderParams.proj = proj; //DirectX::XMMatrixTranspose(proj);
}

/* Draw Call */
void GraphicsVulkan::DrawIndex(int id)
{
	auto memory = m_uniformBuffers[m_imageIndex].memory;
	void* map;
	vkMapMemory(m_device, memory, 0, VK_WHOLE_SIZE, 0, &map);
	memcpy(map, &m_ShaderParams, sizeof(GraphicsVulkan::ShaderParameters));
	vkUnmapMemory(m_device, memory);

	vkCmdBindPipeline(m_commands[m_imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkDeviceSize offset{};
	vkCmdBindVertexBuffers(m_commands[m_imageIndex], 0, 1, &m_vertexBuffers[id].buffer, &offset);
	vkCmdBindIndexBuffer(m_commands[m_imageIndex], m_indexBuffers[id].buffer, offset, VK_INDEX_TYPE_UINT32);

	VkDescriptorSet descriptoSets[]{ m_descriptorSet[m_imageIndex] };
	vkCmdBindDescriptorSets(m_commands[m_imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, descriptoSets, 0, nullptr);

	vkCmdDrawIndexed(m_commands[m_imageIndex], m_indexCounts[id], 1, 0, 0, 0);
}

// Create instance
bool GraphicsVulkan::CreateInstance()
{
	std::vector<const char*> extensionsName;
	VkApplicationInfo appInfo{};
	appInfo.sType			= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion		= VK_API_VERSION_1_3;
	appInfo.engineVersion	= VK_MAKE_VERSION(1, 3, 0);
	appInfo.pApplicationName = "vulkan";

	// Get info of extensions
	std::vector<VkExtensionProperties> properties;
	uint32_t count{};
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	properties.resize(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

	for (const auto& p : properties)
		extensionsName.push_back(p.extensionName);

	extensionsName.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensionsName.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	VkInstanceCreateInfo createInfo{};
	createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount	= uint32_t(extensionsName.size());
	createInfo.ppEnabledExtensionNames	= extensionsName.data();
	createInfo.pApplicationInfo			= &appInfo;

#ifdef _DEBUG 
  // デバッグビルド時には検証レイヤーを有効化
  const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
  if (VK_HEADER_VERSION_COMPLETE < VK_MAKE_VERSION(1, 1, 106)) {
      // "VK_LAYER_LUNARG_standard_validation" は廃止になっているが昔の Vulkan SDK では動くので対処しておく.
      layers[0] = "VK_LAYER_LUNARG_standard_validation";
  }
  createInfo.enabledLayerCount = 1;
  createInfo.ppEnabledLayerNames = layers;
#endif

	VkResult ret = vkCreateInstance(&createInfo, nullptr, &m_instance);
	if (ret != VK_SUCCESS)
		return false;

	return true;
}

// Select physical device
bool GraphicsVulkan::SelectPhysicalDevice()
{
	// Get info of physical devices
	std::vector<VkPhysicalDevice> physicalDevices;
	uint32_t deviceCount{};
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	physicalDevices.resize(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

	// Get a device for the first found
	m_physicalDevice = physicalDevices[0];

	// Get a memory
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemory);

	// Get a graphics queue index
	std::vector<VkQueueFamilyProperties> queueFamiryProperties;
	uint32_t queueFamilyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	queueFamiryProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount,queueFamiryProperties.data());

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamiryProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_graphicsQueueIndex = i;	// Set graphics queue index
			break;
		}
	}

	return true;
}

// Create device
bool GraphicsVulkan::CreateDevice()
{
	const float defaultQueuePriority(1.0f);
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex	= m_graphicsQueueIndex;
	queueCreateInfo.queueCount			= 1;
	queueCreateInfo.pQueuePriorities	= &defaultQueuePriority;

	std::vector<VkExtensionProperties> deviceExtensions;
	uint32_t count{};
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, nullptr);
	deviceExtensions.resize(count);
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &count, deviceExtensions.data());

	std::vector<const char*>extensionNames;
	for (const auto& d : deviceExtensions)
		extensionNames.push_back(d.extensionName);

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos			= &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount		= 1;
	deviceCreateInfo.ppEnabledExtensionNames	= extensionNames.data();
	deviceCreateInfo.enabledExtensionCount		= count;

	VkResult ret = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
	if (ret != VK_SUCCESS)
		return false;

	// Get a queue of m_device
	vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_deviceQueue);

	return true;
}

// Create command pool
bool GraphicsVulkan::CreateCommandPool()
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = m_graphicsQueueIndex;
	createInfo.flags			= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	
	VkResult ret = vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool);
	if (ret != VK_SUCCESS)
		return false;

	return true;
}

// Create window surface for windows
bool GraphicsVulkan::CreateSurface(HWND hWnd)
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType		= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext		= nullptr;
	createInfo.flags		= 0;
	createInfo.hwnd			= hWnd;
	createInfo.hinstance	= (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	VkResult ret = vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
	if (ret != VK_SUCCESS)
		return false;

	// Get a surface capabirity
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_surfaceCapability);

	return true;
}

// Select surface format
bool GraphicsVulkan::SelectSurfaceFormat()
{
	std::vector<VkSurfaceFormatKHR> formats;
	uint32_t surfaceFormatCount{};
	auto ret = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
	if (ret != VK_SUCCESS)
		return false;
	
	formats.resize(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, formats.data());

	for (const auto& f : formats)
	{
		if (f.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM)
		{
			m_surfaceFormat = f;
			break;
		}
	}

	return true;
}

// Create swapchain
bool GraphicsVulkan::CreateSwapchain(const int width, const int height)
{
	unsigned int imageCount = std::max(2u, m_surfaceCapability.minImageCount);
	VkExtent2D extent		= m_surfaceCapability.currentExtent;
	if (extent.width == ~0u)
	{// if extent value invalid then use window size value
		extent.width	= width;
		extent.height	= height;
	}

	uint32_t queueFamilyIndices[]{ m_graphicsQueueIndex };
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface					= m_surface;
	swapchainCreateInfo.minImageCount			= imageCount;
	swapchainCreateInfo.imageFormat				= m_surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace			= m_surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent				= extent;
	swapchainCreateInfo.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform			= m_surfaceCapability.currentTransform;
	swapchainCreateInfo.imageArrayLayers		= 1;
	swapchainCreateInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount	= 1;
	swapchainCreateInfo.pQueueFamilyIndices		= queueFamilyIndices;
	swapchainCreateInfo.presentMode				= VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.oldSwapchain			= VK_NULL_HANDLE;
	swapchainCreateInfo.clipped					= VK_TRUE;
	swapchainCreateInfo.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkResult ret = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
	if (ret != VK_SUCCESS)
		return false;

	m_swapchainImages.resize(imageCount);
	m_swapchainViews.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

	// Creates iamge view
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType			= VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format			= m_surfaceFormat.format;
		createInfo.components		= { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
		createInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		createInfo.image			= m_swapchainImages[i];

		ret = vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainViews[i]);
		if (ret != VK_SUCCESS)
			return false;
	}

	m_swapchainExtent = extent;

	return true;
}

// Create depth buffer
bool GraphicsVulkan::CreateDepthBuffer()
{
	VkImageCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType		= VK_IMAGE_TYPE_2D;
	createInfo.format			= VK_FORMAT_D32_SFLOAT;
	createInfo.extent.width		= m_swapchainExtent.width;
	createInfo.extent.height	= m_swapchainExtent.height;
	createInfo.extent.depth		= 1;
	createInfo.mipLevels		= 1;
	createInfo.usage			= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	createInfo.samples			= VK_SAMPLE_COUNT_1_BIT;
	createInfo.arrayLayers		= 1;

	VkResult ret = vkCreateImage(m_device, &createInfo, nullptr, &m_depthBuffer);
	if (ret != VK_SUCCESS)
		return false;

	VkMemoryRequirements request{};
	vkGetImageMemoryRequirements(m_device, m_depthBuffer, &request);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize		= request.size;
	allocateInfo.memoryTypeIndex	= getMemoryTypeIndex(request.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	ret	= vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_depthBufferMemory);
	if (ret != VK_SUCCESS)
		return false;

	ret = vkBindImageMemory(m_device, m_depthBuffer, m_depthBufferMemory, 0);
	if (ret != VK_SUCCESS)
		return false;

	// Create image view
	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType			= VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format			= VK_FORMAT_D32_SFLOAT;
	viewCreateInfo.components		= { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G , VK_COMPONENT_SWIZZLE_B , VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
	viewCreateInfo.image			= m_depthBuffer;

	ret = vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_depthBufferView);
	if (ret != VK_SUCCESS)
		return false;

	return true;
}

// Create render pass
bool GraphicsVulkan::CreateRenderPass()
{
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	std::array<VkAttachmentDescription, 2> attachments;
	VkAttachmentDescription& colorTarget = attachments[0];
	VkAttachmentDescription& depthTarget = attachments[1];

	colorTarget = {};
	colorTarget.format			= m_surfaceFormat.format;
	colorTarget.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorTarget.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorTarget.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorTarget.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorTarget.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorTarget.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorTarget.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	depthTarget = {};
	depthTarget.format			= VK_FORMAT_D32_SFLOAT;
	depthTarget.samples			= VK_SAMPLE_COUNT_1_BIT;
	depthTarget.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTarget.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	depthTarget.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTarget.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTarget.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	depthTarget.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef{}, depthRef{};
	colorRef.attachment = 0;
	colorRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	depthRef.attachment = 1;
	depthRef.layout		= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPassDesc{};
	subPassDesc.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPassDesc.colorAttachmentCount	= 1;
	subPassDesc.pColorAttachments		= &colorRef;
	subPassDesc.pDepthStencilAttachment = &depthRef;

	createInfo.attachmentCount	= uint32_t(attachments.size());
	createInfo.pAttachments		= attachments.data();
	createInfo.subpassCount		= 1;
	createInfo.pSubpasses		= &subPassDesc;

	// Create render pass
	VkResult ret = vkCreateRenderPass(m_device, &createInfo, nullptr, &m_renderPass);
	if (ret != VK_SUCCESS)
		return false;

	return true;
}

// Create frame buffer
bool GraphicsVulkan::CreateFrameBuffer()
{
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType		= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass	= m_renderPass;
	createInfo.width		= m_swapchainExtent.width;
	createInfo.height		= m_swapchainExtent.height;
	createInfo.layers		= 1;
	m_frameBuffers.resize(m_swapchainViews.size());

	for (size_t i = 0; i < m_swapchainViews.size(); ++i)
	{
		std::array<VkImageView, 2> attachments;
		attachments[0]				= m_swapchainViews[i];
		attachments[1]				= m_depthBufferView;
		createInfo.attachmentCount	= uint32_t(attachments.size());
		createInfo.pAttachments		= attachments.data();

		VkResult ret = vkCreateFramebuffer(m_device, &createInfo, nullptr, &m_frameBuffers[i]);
		if (ret != VK_SUCCESS)
			return false;
	}

	return true;
}

// Create fence
bool GraphicsVulkan::CreateFence()
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool		= m_commandPool;
	allocateInfo.commandBufferCount = uint32_t(m_swapchainViews.size());
	allocateInfo.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	m_commands.resize(allocateInfo.commandBufferCount);

	VkResult ret = vkAllocateCommandBuffers(m_device, &allocateInfo, m_commands.data());
	if (ret != VK_SUCCESS)
		return false;

	m_fences.resize(allocateInfo.commandBufferCount);

	VkFenceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (auto& f : m_fences)
	{
		ret = vkCreateFence(m_device, &createInfo, nullptr, &f);
		if (ret != VK_SUCCESS)
			return false;
	}

	return true;
}

// Create semaphores
bool GraphicsVulkan::CreateSemaphores()
{
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult ret = vkCreateSemaphore(m_device, &createInfo, nullptr, &m_renderSem);
	if (ret != VK_SUCCESS)
		return false;

	ret = vkCreateSemaphore(m_device, &createInfo, nullptr, &m_presentSem);
	if (ret != VK_SUCCESS)
		return false;

	return true;
}

/* Create pipeline */
bool GraphicsVulkan::CreatePipeline()
{
	// Set to vertex layout
	VkVertexInputBindingDescription inputBinding{};
	inputBinding.binding	= 0;
	inputBinding.stride		= sizeof(Vertex3D);
	inputBinding.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 3> inputAttributes
	{
		{
			{0, 0, VK_FORMAT_R32G32B32_SFLOAT,	offsetof(Vertex3D, Position)},
			{1, 0, VK_FORMAT_R32G32B32_SFLOAT,	offsetof(Vertex3D, Normal)},
			{2, 0, VK_FORMAT_R32G32_SFLOAT,		offsetof(Vertex3D, TexCoord)},
		}
	};

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
	vertexInputCreateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount		= 1;
	vertexInputCreateInfo.pVertexBindingDescriptions		= &inputBinding;
	vertexInputCreateInfo.vertexAttributeDescriptionCount	= uint32_t(inputAttributes.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions		= inputAttributes.data();

	// Set to blend state
	const int colorWriteAll = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = true;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.colorBlendOp		= VK_BLEND_OP_ADD;
	blendAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;
	blendAttachment.colorWriteMask		= colorWriteAll;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
	colorBlendCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.attachmentCount	= 1;
	colorBlendCreateInfo.pAttachments		= &blendAttachment;

	// Set to viewport
	VkViewport viewport{};
	viewport.width		= float(m_swapchainExtent.width);
	viewport.height		= float(m_swapchainExtent.height) * -1.0f;
	viewport.y			= float(m_swapchainExtent.height);
	viewport.maxDepth	= 1.0f;

	// Set to scissor rect
	VkRect2D scissor{};
	scissor.offset = {};
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo{};
	viewportCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount	= 1;
	viewportCreateInfo.pViewports		= &viewport;
	viewportCreateInfo.scissorCount		= 1;
	viewportCreateInfo.pScissors		= &scissor;

	// Set to primitive topology
	VkPipelineInputAssemblyStateCreateInfo inputAssenblyCreateInfo{};
	inputAssenblyCreateInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssenblyCreateInfo.topology	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Set to rasterizer state
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
	rasterizerCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.polygonMode	= VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.cullMode		= VK_CULL_MODE_NONE;
	rasterizerCreateInfo.frontFace		= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.lineWidth		= 1.0f;

	// Set to multisample
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;

	// Set to depth stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable		= true;
	depthStencilCreateInfo.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCreateInfo.depthWriteEnable		= true;
	depthStencilCreateInfo.stencilTestEnable	= false;

	// Load to shader binary
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages
	{
		LoadShaderModule("vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		LoadShaderModule("frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
	};

	// Layout of pipeline
	VkPipelineLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = 1;
	layoutCreateInfo.pSetLayouts	= &m_descriptorLayout;
	VkResult ret = vkCreatePipelineLayout(m_device, &layoutCreateInfo, nullptr, &m_pipelineLayout);
	if (ret != VK_SUCCESS)
		return false;

	// Create pipeline
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount			= uint32_t(shaderStages.size());
	pipelineCreateInfo.pStages				= shaderStages.data();
	pipelineCreateInfo.pInputAssemblyState	= &inputAssenblyCreateInfo;
	pipelineCreateInfo.pVertexInputState	= &vertexInputCreateInfo;
	pipelineCreateInfo.pRasterizationState	= &rasterizerCreateInfo;
	pipelineCreateInfo.pDepthStencilState	= &depthStencilCreateInfo;
	pipelineCreateInfo.pMultisampleState	= &multisampleCreateInfo;
	pipelineCreateInfo.pViewportState		= &viewportCreateInfo;
	pipelineCreateInfo.pColorBlendState		= &colorBlendCreateInfo;
	pipelineCreateInfo.renderPass			= m_renderPass;
	pipelineCreateInfo.layout				= m_pipelineLayout;
	ret = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
	if (ret != VK_SUCCESS)
		return false;

	for (const auto& s : shaderStages)
		vkDestroyShaderModule(m_device, s.module, nullptr);

	return true;
}

// Create constant buffer
bool GraphicsVulkan::CreateConstantBuffer()
{
	m_uniformBuffers.resize(m_swapchainViews.size());
	for (auto& v : m_uniformBuffers)
	{
		VkMemoryPropertyFlags uboFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		v = CreateBuffer(sizeof(GraphicsVulkan::ShaderParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uboFlags);
	}

	// Create constant buffer
	VkDescriptorSetLayoutBinding binding{};
	binding.binding			= 0;
	binding.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.stageFlags		= VK_SHADER_STAGE_VERTEX_BIT;
	binding.descriptorCount	= 1;

	// Create layout
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
	layoutCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount	= 1;
	layoutCreateInfo.pBindings		= &binding;
	VkResult ret = vkCreateDescriptorSetLayout(m_device, &layoutCreateInfo, nullptr, &m_descriptorLayout);
	if (ret != VK_SUCCESS)
		return false;

	// Create pool
	VkDescriptorPoolSize descPoolSize;
	descPoolSize.descriptorCount	= uint32_t(m_uniformBuffers.size());
	descPoolSize.type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets			= uint32_t(m_uniformBuffers.size());
	poolCreateInfo.poolSizeCount	= 1;
	poolCreateInfo.pPoolSizes		= &descPoolSize;
	ret = vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_descriptorPool);
	if (ret != VK_SUCCESS)
		return false;

	std::vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i<int(m_uniformBuffers.size()); ++i)
	{
		layouts.push_back(m_descriptorLayout);
	}
	VkDescriptorSetAllocateInfo ai{};
	ai.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool		= m_descriptorPool;
	ai.descriptorSetCount	= uint32_t(m_uniformBuffers.size());
	ai.pSetLayouts			= layouts.data();
	m_descriptorSet.resize(m_uniformBuffers.size());
	ret = vkAllocateDescriptorSets(m_device, &ai, m_descriptorSet.data());
	if (ret != VK_SUCCESS)
		return false;

	// ディスクリプタセットへ書き込み.
	for (int i = 0; i<int(m_uniformBuffers.size()); ++i)
	{
		VkDescriptorBufferInfo descUBO{};
		descUBO.buffer = m_uniformBuffers[i].buffer;
		descUBO.offset = 0;
		descUBO.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet ubo{};
		ubo.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ubo.dstBinding		= 0;
		ubo.descriptorCount = 1;
		ubo.descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.pBufferInfo		= &descUBO;
		ubo.dstSet			= m_descriptorSet[i];						// モデルそれぞれにVkDescriptorSetが必要

		vkUpdateDescriptorSets(m_device, 1, &ubo, 0, nullptr);
	}

	return true;
}

// Create buffer object
GraphicsVulkan::BufferObject GraphicsVulkan::CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
{
	BufferObject obj{};
	VkBufferCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.usage	= usage;
	createInfo.size		= size;
	VkResult ret = vkCreateBuffer(m_device, &createInfo, nullptr, &obj.buffer);
	if (ret != VK_SUCCESS)
		return BufferObject();

	VkMemoryRequirements request{};
	vkGetBufferMemoryRequirements(m_device, obj.buffer, &request);
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = request.size;
	
	allocateInfo.memoryTypeIndex = getMemoryTypeIndex(request.memoryTypeBits, flags);
	ret = vkAllocateMemory(m_device, &allocateInfo, nullptr, &obj.memory);
	if (ret != VK_SUCCESS)
		return BufferObject();

	ret = vkBindBufferMemory(m_device, obj.buffer, obj.memory, 0);
	if (ret != VK_SUCCESS)
		return BufferObject();

	return obj;
}

// Load shader
VkPipelineShaderStageCreateInfo GraphicsVulkan::LoadShaderModule(const char* fileName, VkShaderStageFlagBits stage)
{
	std::ifstream ifs(fileName, std::ios::binary);
	if (ifs.fail())
		return VkPipelineShaderStageCreateInfo();

	std::vector<char> fileData;
	ifs.seekg(0, std::wifstream::end);
	fileData.resize(ifs.tellg());
	ifs.seekg(0, std::wifstream::beg);

	ifs.read(fileData.data(), fileData.size());

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode	= (uint32_t*)fileData.data();
	createInfo.codeSize = fileData.size();
	VkResult ret = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
	if (ret != VK_SUCCESS)
		return VkPipelineShaderStageCreateInfo();

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
	shaderStageCreateInfo.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderStageCreateInfo.stage		= stage;
	shaderStageCreateInfo.module	= shaderModule;
	shaderStageCreateInfo.pName		= "main";
	return shaderStageCreateInfo;
}

// Get memory type index
uint32_t GraphicsVulkan::getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProperty) const
{
	uint32_t ret = ~0u;
	for (uint32_t i = 0; i < m_physicalDeviceMemory.memoryTypeCount; ++i)
	{
		if (requestBits & 1)
		{
			const auto& types = m_physicalDeviceMemory.memoryTypes[i];
			if ((types.propertyFlags & requestProperty) == requestProperty)
			{
				ret = i;
				break;
			}
		}
		requestBits >>= 1;
	}

	return ret;
}
