/*===================================================================================
*	Date : 2022/10/21(Fri)
*		Author	: Gakuto.S
*		File	: Graphics_Vulkan.cpp
*		Detail	:
===================================================================================*/
#include <sstream>

#include "Graphics_Vulkan.h"

#define GetInstanceProcAddr(FuncName) \
  m_##FuncName = reinterpretcast<PFN##FuncName>(vkGetInstanceProcAddr(m_instance, #FuncName))

using namespace std;

//static VkBool32 VKAPI_CALL DebugReportCallback(
//	VkDebugReportFlagsEXT flags,
//	VkDebugReportObjectTypeEXT objactTypes,
//	uint64_t object,
//	size_t    location,
//	int32_t messageCode,
//	const char* pLayerPrefix,
//	const char* pMessage,
//	void* pUserData)
//{
//	VkBool32 ret = VK_FALSE;
//	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ||
//		flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//	{
//		ret = VK_TRUE;
//	}
//	std::stringstream ss;
//	if (pLayerPrefix)
//	{
//		ss << "[" << pLayerPrefix << "] ";
//	}
//	ss << pMessage << std::endl;
//
//	OutputDebugStringA(ss.str().c_str());
//
//	return ret;
//}

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

	return true;
}

/* Uninitialize */
void GraphicsVulkan::Uninit()
{
	vkDeviceWaitIdle(m_device);
	
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

int GraphicsVulkan::CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataSize, const unsigned int* iData, size_t iDataSize)
{
	return 0;
}

void GraphicsVulkan::SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl)
{
}

void GraphicsVulkan::SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up)
{
}

void GraphicsVulkan::SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ)
{
}

void GraphicsVulkan::DrawIndex(int id)
{
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
	VkAttachmentDescription colorTarget = attachments[0];
	VkAttachmentDescription depthTarget = attachments[1];

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

// Get memory type index
uint32_t GraphicsVulkan::getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlagBits requestProperty) const
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
