/*===================================================================================
*	Date : 2022/10/21(Fri)
*		Author	: Gakuto.S
*		File	: Graphics_Vulkan.cpp
*		Detail	:
===================================================================================*/
#include "Graphics_Vulkan.h"

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

	return true;
}

/* Uninitialize */
void GraphicsVulkan::Uninit()
{
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void GraphicsVulkan::Clear()
{
}

void GraphicsVulkan::Present()
{
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
	appInfo.engineVersion	= VK_MAKE_VERSION(1, 0, 3);


	// Get info of extensions
	std::vector<VkExtensionProperties> properties;
	uint32_t count{};
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	properties.resize(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

	for (const auto& p : properties)
		extensionsName.push_back(p.extensionName);

	VkInstanceCreateInfo createInfo{};
	createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount	= count;
	createInfo.ppEnabledExtensionNames	= extensionsName.data();
	createInfo.pApplicationInfo			= &appInfo;

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

	uint32_t graphicsQueue = ~0u;
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
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
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

// Create swap chain
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
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface					= m_surface;
	createInfo.minImageCount			= imageCount;
	createInfo.imageFormat				= m_surfaceFormat.format;
	createInfo.imageColorSpace			= m_surfaceFormat.colorSpace;
	createInfo.imageExtent				= extent;
	createInfo.imageUsage				= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform				= m_surfaceCapability.currentTransform;
	createInfo.imageArrayLayers			= 1;
	createInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount	= 0;
	createInfo.presentMode				= VK_PRESENT_MODE_FIFO_KHR;
	createInfo.oldSwapchain				= VK_NULL_HANDLE;
	createInfo.clipped					= VK_TRUE;
	createInfo.compositeAlpha			= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	VkResult ret = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain);
	if (ret != VK_SUCCESS)
		return false;

	m_swapchainExtent = extent;

	return true;
}
