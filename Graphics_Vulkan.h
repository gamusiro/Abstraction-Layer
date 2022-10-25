/*===================================================================================
*	Date : 2022/10/21(Fri)
*		Author	: Gakuto.S
*		File	: Graphics_Vulkan.h
*		Detail	:
===================================================================================*/
#pragma once
#include <vector>
#include <array>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>
#pragma comment(lib, "vulkan-1.lib")


#include "Graphics_Interface.h"

class GraphicsVulkan : public IGraphics
{
private:
	struct BufferObject
	{
		VkBuffer		buffer;
		VkDeviceMemory	memory;
	};

public:
	bool Init(int width, int height, void* handle);
	void Uninit();
	void Clear();
	void Present();
	int	 CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataSize, const unsigned int* iData, size_t iDataSize);
	int	 CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index);
	void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl);
	void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up);
	void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ);
	void DrawIndex(int id);

private:
	bool CreateInstance();
	bool SelectPhysicalDevice();
	bool CreateDevice();
	bool CreateCommandPool();
	bool CreateSurface(HWND hWnd);
	bool SelectSurfaceFormat();
	bool CreateSwapchain(const int width, const int height);
	bool CreateDepthBuffer();
	bool CreateRenderPass();
	bool CreateFrameBuffer();
	bool CreateFence();
	bool CreateSemaphores();
	bool CreatePipeline();
	bool CreateConstantBuffer();

	BufferObject					CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VkPipelineShaderStageCreateInfo LoadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

	uint32_t getMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProperty) const;

private:
	VkInstance							m_instance;				// Vulakn instance
	VkPhysicalDevice					m_physicalDevice;		// Vulkam physical device
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemory;	// Vulkan physical device memory properties
	uint32_t							m_graphicsQueueIndex;	// Vulkan graphics queue index
	VkDevice							m_device;				// Vulkan logical device
	VkQueue								m_deviceQueue;			// Vulkan device queue
	VkCommandPool						m_commandPool;			// Vulkan command pool
	VkSurfaceKHR						m_surface;				// Vulakn surface
	VkSurfaceCapabilitiesKHR			m_surfaceCapability;	// Vulkan surface capability
	VkSurfaceFormatKHR					m_surfaceFormat;		// Vulakn surface format
	VkSwapchainKHR						m_swapchain;			// Vulkan swapchain
	VkExtent2D							m_swapchainExtent;		// Vulkan swapchain extent
	std::vector<VkImage>				m_swapchainImages;		// Vulkan swapchain images
	std::vector<VkImageView>			m_swapchainViews;		// Vulkan swapchain image views
	VkImage								m_depthBuffer;			// Vulkan depth buffer
	VkDeviceMemory						m_depthBufferMemory;	// Vulkan depth buffer memory
	VkImageView							m_depthBufferView;		// Vulkan depth buffer view
	VkRenderPass						m_renderPass;			// Vulkan render pass
	std::vector<VkFramebuffer>			m_frameBuffers;			// Vulkan frame buffers
	std::vector<VkFence>				m_fences;				// Vulkan fence
	VkSemaphore							m_renderSem;			// Vulakn Catch the event of render completed
	VkSemaphore							m_presentSem;			// Vulakn Catch the event of present completed
	std::vector<VkCommandBuffer>		m_commands;				// Vulkan command buffers

	uint32_t							m_imageIndex;			// Vulkan currently frame index of swapchain

	std::vector<BufferObject>			m_vertexBuffers;		// Vulkan vertex buffers
	std::vector<BufferObject>			m_indexBuffers;			// Vulkan index buffers
	std::vector<size_t>					m_indexCounts;			// Vulkan index counts

	VkPipelineLayout					m_pipelineLayout;		// Vulkan pipeline layout
	VkPipeline							m_pipeline;				// Vulkan pipeline

	struct UniformBuffer
	{
		std::vector<BufferObject>		buffer;
		VkDescriptorPool				pool;
		std::vector<VkDescriptorSet>	sets;
	};

	std::vector<UniformBuffer>			m_uniformBuffers;
	VkDescriptorSetLayout				m_descriptorLayout;		//
	std::vector<VkDescriptorSet>		m_descriptorSets;

	//std::vector<BufferObject>			m_uniformBuffers;		//
	//VkDescriptorPool					m_descriptorPool;
	//std::vector<VkDescriptorSet>		m_descriptorSet;

	DirectX::XMMATRIX					m_world;
	DirectX::XMMATRIX					m_view;
	DirectX::XMMATRIX					m_proj;
};

