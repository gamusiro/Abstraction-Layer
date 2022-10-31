/*===================================================================================
*	Date : 2022/10/21(Fri)
*		Author	: Gakuto.S
*		File	: Graphics_Vulkan.h
*		Detail	:
===================================================================================*/
#pragma once
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
	//**************************************************
	/// \brief Constructor
	/// 
	/// \return none
	//**************************************************
	GraphicsVulkan()
		:m_instance(),
		m_physicalDevice(),
		m_physicalDeviceMemory(),
		m_graphicsQueueIndex(),
		m_device(),
		m_deviceQueue(),
		m_commandPool(),
		m_surface(),
		m_surfaceCapability(),
		m_surfaceFormat(),
		m_swapchain(),
		m_swapchainExtent(),
		m_depthBuffer(),
		m_depthBufferMemory(),
		m_depthBufferView(),
		m_renderPass(),
		m_renderSem(),
		m_presentSem(),
		m_imageIndex(),
		m_pipelineLayout(),
		m_pipeline(),
		m_descriptorLayout(),
		m_view(),
		m_proj()
	{}

	//**************************************************
	/// \brief Initialize Vulkan
	/// 
	/// \return Success is true
	//**************************************************
	bool Init(int width, int height, void* handle);
	
	//**************************************************
	/// \brief Uninitialize Vulkan
	/// 
	/// \return none
	//**************************************************
	void Uninit();

	//**************************************************
	/// \brief Clear screen
	/// 
	/// \return none
	//**************************************************
	void Clear();

	//**************************************************
	/// \brief Present buffer
	/// 
	/// \return none
	//**************************************************
	void Present();

	//**************************************************
	/// \brief Create buffer
	/// 
	/// \param[in] vData	-> model vertex data
	/// \param[in] vDataNum	-> count vertex
	/// \param[in] iData	-> model index data
	/// \param[in] iDataNum	-> count index
	/// 
	/// \return control id
	//**************************************************
	int	 CreateVertexBufferAndIndexBuffer(const structure::Vertex3D* vData, size_t vDataSize, const unsigned int* iData, size_t iDataSize);
	
	//**************************************************
	/// \brief Create matrix buffer
	/// 
	/// \return control id
	//**************************************************
	int	 CreateMatrixBuffer(CONSTANT_BUFFER_INDEX index);
	
	//**************************************************
	/// \brief Set world matrix
	/// 
	/// \param[in] pos -> position of model in 3d space
	/// \param[in] rot -> angle of model 
	/// \param[in] pos -> scale of model
	/// 
	/// \return none
	//**************************************************
	void SetWorldMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 rot, const DirectX::XMFLOAT3 scl);
	
	//**************************************************
	/// \brief Set view matrix
	/// 
	/// \param[in] pos		-> position of camera in 3d space
	/// \param[in] target	-> position of focus in 3d space 
	/// \param[in] up		-> up
	/// 
	/// \return none
	//**************************************************
	void SetViewMatrix(int id, const DirectX::XMFLOAT3 pos, const DirectX::XMFLOAT3 target, const DirectX::XMFLOAT3 up);
	
	//**************************************************
	/// \brief Set projection matrix
	/// 
	/// \param[in] fov
	/// \param[in] aspect
	/// \param[in] nearZ
	/// \param[in] farZ
	/// 
	/// \return none
	//**************************************************
	void SetProjectionMatrix(int id, float fov, float aspect, float nearZ, float farZ);

	//**************************************************
	/// \brief Set vertex buffer and index buffer
	/// 
	/// \param[in] control id
	/// 
	/// \return none
	//**************************************************
	void DrawIndex(int id);

private:
	//**************************************************
	/// \brief Create instance
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateInstance();
	
	//**************************************************
	/// \brief Select Physical device
	/// 
	/// \return Succcess is true
	//**************************************************
	bool SelectPhysicalDevice();

	//**************************************************
	/// \brief Create logical device
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateDevice();

	//**************************************************
	/// \brief Create command pool
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateCommandPool();

	//**************************************************
	/// \brief Create surface
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateSurface(HWND hWnd);

	//**************************************************
	/// \brief Select surface format
	/// 
	/// \return Succcess is true
	//**************************************************
	bool SelectSurfaceFormat();

	//**************************************************
	/// \brief Create swap chain
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateSwapchain(const int width, const int height);
	
	//**************************************************
	/// \brief Create depth buffer
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateDepthBuffer();

	//**************************************************
	/// \brief Create render pass
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateRenderPass();

	//**************************************************
	/// \brief Create frame buffer
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateFrameBuffer();

	//**************************************************
	/// \brief Create fence
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateFence();

	//**************************************************
	/// \brief Create semaphores
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateSemaphores();

	//**************************************************
	/// \brief Create pipeline
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreatePipeline();

	//**************************************************
	/// \brief Create constant buffer
	/// 
	/// \return Succcess is true
	//**************************************************
	bool CreateConstantBuffer();

	//**************************************************
	/// \brief Create buffer object
	/// 
	/// \return Buffer object
	//**************************************************
	BufferObject					CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	
	//**************************************************
	/// \brief Load shader of spv file
	/// 
	/// \return VkPipelineShaderStageCreateInfo
	//**************************************************
	VkPipelineShaderStageCreateInfo LoadShaderModule(const char* fileName, VkShaderStageFlagBits stage);

	//**************************************************
	/// \brief Get memory type
	/// 
	/// \return memory type index
	//**************************************************
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
		std::vector<BufferObject>		buffer;					// constant buffer objects
		VkDescriptorPool				pool;					// Vulkan descriptor pool 
		std::vector<VkDescriptorSet>	sets;					// Vulkan descriptor sets
	};

	std::vector<UniformBuffer>			m_uniformBuffers;		// Constant buffers
	VkDescriptorSetLayout				m_descriptorLayout;		// Vulkan descriptor layout


	DirectX::XMMATRIX					m_view;
	DirectX::XMMATRIX					m_proj;
};

