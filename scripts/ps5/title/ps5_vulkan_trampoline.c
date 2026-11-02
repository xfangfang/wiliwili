/* Generated PS5 Vulkan Trampoline - dispatches statically to Mesa RADV */
#include <vulkan/vulkan.h>
#include <stddef.h>

/* CPU feature model for compiler builtins (__builtin_cpu_supports) in addrlib */
struct __ps5_cpu_model {
    unsigned int __cpu_vendor;
    unsigned int __cpu_type;
    unsigned int __cpu_subtype;
    unsigned int __cpu_features[4];
};

struct __ps5_cpu_model __cpu_model = {
    2, /* AMD */
    0,
    0,
    { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff } /* All features (including AVX2) enabled for Zen 2 */
};

int __cpu_indicator_init(void) {
    return 0;
}

#include <stdint.h>
static int32_t s_videoout_handle = -1;
extern int __real_sceVideoOutOpen(int32_t user, int32_t bus, int32_t index, const void *parameter);

int __wrap_sceVideoOutOpen(int32_t user, int32_t bus, int32_t index, const void *parameter) {
    int handle = __real_sceVideoOutOpen(user, bus, index, parameter);
    if (handle >= 0) {
        s_videoout_handle = handle;
    }
    return handle;
}

int32_t ps5_get_videoout_handle(void) {
    return s_videoout_handle;
}

extern PFN_vkVoidFunction vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName);

static VkInstance s_vkInstance = VK_NULL_HANDLE;

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    return vk_icdGetInstanceProcAddr(instance ? instance : s_vkInstance, pName);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    static PFN_vkCreateInstance pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateInstance)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateInstance");
    VkResult res = pfn(pCreateInfo, pAllocator, pInstance);
    if (res == VK_SUCCESS && pInstance && *pInstance) s_vkInstance = *pInstance;
    return res;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyInstance pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyInstance)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyInstance");
    if (pfn) pfn(instance, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    static PFN_vkEnumeratePhysicalDevices pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDevices)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDevices");
    return pfn ? pfn(instance, pPhysicalDeviceCount, pPhysicalDevices) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    static PFN_vkGetPhysicalDeviceFeatures pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFeatures)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFeatures");
    if (pfn) pfn(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    static PFN_vkGetPhysicalDeviceFormatProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFormatProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFormatProperties");
    if (pfn) pfn(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceImageFormatProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceImageFormatProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceImageFormatProperties");
    return pfn ? pfn(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    static PFN_vkGetPhysicalDeviceProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceProperties");
    if (pfn) pfn(physicalDevice, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties");
    if (pfn) pfn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    static PFN_vkGetPhysicalDeviceMemoryProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceMemoryProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceMemoryProperties");
    if (pfn) pfn(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    static PFN_vkGetDeviceProcAddr pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceProcAddr)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceProcAddr");
    return pfn ? pfn(device, pName) : (PFN_vkVoidFunction)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    static PFN_vkCreateDevice pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDevice)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDevice");
    return pfn ? pfn(physicalDevice, pCreateInfo, pAllocator, pDevice) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDevice pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDevice)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDevice");
    if (pfn) pfn(device, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    static PFN_vkEnumerateInstanceExtensionProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumerateInstanceExtensionProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumerateInstanceExtensionProperties");
    return pfn ? pfn(pLayerName, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    static PFN_vkEnumerateDeviceExtensionProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumerateDeviceExtensionProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumerateDeviceExtensionProperties");
    return pfn ? pfn(physicalDevice, pLayerName, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    static PFN_vkEnumerateInstanceLayerProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumerateInstanceLayerProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumerateInstanceLayerProperties");
    return pfn ? pfn(pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    static PFN_vkEnumerateDeviceLayerProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumerateDeviceLayerProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumerateDeviceLayerProperties");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    static PFN_vkGetDeviceQueue pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceQueue)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceQueue");
    if (pfn) pfn(device, queueFamilyIndex, queueIndex, pQueue);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    static PFN_vkQueueSubmit pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueSubmit)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueSubmit");
    return pfn ? pfn(queue, submitCount, pSubmits, fence) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(VkQueue queue) {
    static PFN_vkQueueWaitIdle pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueWaitIdle)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueWaitIdle");
    return pfn ? pfn(queue) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(VkDevice device) {
    static PFN_vkDeviceWaitIdle pfn = NULL;
    if (!pfn) pfn = (PFN_vkDeviceWaitIdle)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDeviceWaitIdle");
    return pfn ? pfn(device) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    static PFN_vkAllocateMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkAllocateMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAllocateMemory");
    return pfn ? pfn(device, pAllocateInfo, pAllocator, pMemory) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkFreeMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkFreeMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkFreeMemory");
    if (pfn) pfn(device, memory, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    static PFN_vkMapMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkMapMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkMapMemory");
    return pfn ? pfn(device, memory, offset, size, flags, ppData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    static PFN_vkUnmapMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkUnmapMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUnmapMemory");
    if (pfn) pfn(device, memory);
}

VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    static PFN_vkFlushMappedMemoryRanges pfn = NULL;
    if (!pfn) pfn = (PFN_vkFlushMappedMemoryRanges)vk_icdGetInstanceProcAddr(s_vkInstance, "vkFlushMappedMemoryRanges");
    return pfn ? pfn(device, memoryRangeCount, pMemoryRanges) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    static PFN_vkInvalidateMappedMemoryRanges pfn = NULL;
    if (!pfn) pfn = (PFN_vkInvalidateMappedMemoryRanges)vk_icdGetInstanceProcAddr(s_vkInstance, "vkInvalidateMappedMemoryRanges");
    return pfn ? pfn(device, memoryRangeCount, pMemoryRanges) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    static PFN_vkGetDeviceMemoryCommitment pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceMemoryCommitment)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceMemoryCommitment");
    if (pfn) pfn(device, memory, pCommittedMemoryInBytes);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    static PFN_vkBindBufferMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindBufferMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindBufferMemory");
    return pfn ? pfn(device, buffer, memory, memoryOffset) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    static PFN_vkBindImageMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindImageMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindImageMemory");
    return pfn ? pfn(device, image, memory, memoryOffset) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    static PFN_vkGetBufferMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferMemoryRequirements");
    if (pfn) pfn(device, buffer, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    static PFN_vkGetImageMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageMemoryRequirements");
    if (pfn) pfn(device, image, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    static PFN_vkGetImageSparseMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSparseMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSparseMemoryRequirements");
    if (pfn) pfn(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    static PFN_vkGetPhysicalDeviceSparseImageFormatProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSparseImageFormatProperties");
    if (pfn) pfn(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    static PFN_vkQueueBindSparse pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueBindSparse)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueBindSparse");
    return pfn ? pfn(queue, bindInfoCount, pBindInfo, fence) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    static PFN_vkCreateFence pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateFence)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateFence");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pFence) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyFence pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyFence)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyFence");
    if (pfn) pfn(device, fence, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    static PFN_vkResetFences pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetFences)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetFences");
    return pfn ? pfn(device, fenceCount, pFences) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(VkDevice device, VkFence fence) {
    static PFN_vkGetFenceStatus pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetFenceStatus)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetFenceStatus");
    return pfn ? pfn(device, fence) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    static PFN_vkWaitForFences pfn = NULL;
    if (!pfn) pfn = (PFN_vkWaitForFences)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWaitForFences");
    return pfn ? pfn(device, fenceCount, pFences, waitAll, timeout) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    static PFN_vkCreateSemaphore pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSemaphore)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSemaphore");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSemaphore) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySemaphore pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySemaphore)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySemaphore");
    if (pfn) pfn(device, semaphore, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    static PFN_vkCreateQueryPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateQueryPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateQueryPool");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pQueryPool) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyQueryPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyQueryPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyQueryPool");
    if (pfn) pfn(device, queryPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    static PFN_vkGetQueryPoolResults pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetQueryPoolResults)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetQueryPoolResults");
    return pfn ? pfn(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    static PFN_vkCreateBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateBuffer");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pBuffer) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyBuffer");
    if (pfn) pfn(device, buffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    static PFN_vkCreateImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateImage");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pImage) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyImage");
    if (pfn) pfn(device, image, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    static PFN_vkGetImageSubresourceLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSubresourceLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSubresourceLayout");
    if (pfn) pfn(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    static PFN_vkCreateImageView pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateImageView)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateImageView");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pView) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyImageView pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyImageView)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyImageView");
    if (pfn) pfn(device, imageView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    static PFN_vkCreateCommandPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateCommandPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateCommandPool");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pCommandPool) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyCommandPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyCommandPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyCommandPool");
    if (pfn) pfn(device, commandPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    static PFN_vkResetCommandPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetCommandPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetCommandPool");
    return pfn ? pfn(device, commandPool, flags) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    static PFN_vkAllocateCommandBuffers pfn = NULL;
    if (!pfn) pfn = (PFN_vkAllocateCommandBuffers)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAllocateCommandBuffers");
    return pfn ? pfn(device, pAllocateInfo, pCommandBuffers) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    static PFN_vkFreeCommandBuffers pfn = NULL;
    if (!pfn) pfn = (PFN_vkFreeCommandBuffers)vk_icdGetInstanceProcAddr(s_vkInstance, "vkFreeCommandBuffers");
    if (pfn) pfn(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    static PFN_vkBeginCommandBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkBeginCommandBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBeginCommandBuffer");
    return pfn ? pfn(commandBuffer, pBeginInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer commandBuffer) {
    static PFN_vkEndCommandBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkEndCommandBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEndCommandBuffer");
    return pfn ? pfn(commandBuffer) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    static PFN_vkResetCommandBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetCommandBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetCommandBuffer");
    return pfn ? pfn(commandBuffer, flags) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    static PFN_vkCmdCopyBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBuffer");
    if (pfn) pfn(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    static PFN_vkCmdCopyImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImage");
    if (pfn) pfn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    static PFN_vkCmdCopyBufferToImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBufferToImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBufferToImage");
    if (pfn) pfn(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    static PFN_vkCmdCopyImageToBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImageToBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImageToBuffer");
    if (pfn) pfn(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    static PFN_vkCmdUpdateBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdUpdateBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdUpdateBuffer");
    if (pfn) pfn(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    static PFN_vkCmdFillBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdFillBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdFillBuffer");
    if (pfn) pfn(commandBuffer, dstBuffer, dstOffset, size, data);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    static PFN_vkCmdPipelineBarrier pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPipelineBarrier)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPipelineBarrier");
    if (pfn) pfn(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    static PFN_vkCmdBeginQuery pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginQuery)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginQuery");
    if (pfn) pfn(commandBuffer, queryPool, query, flags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    static PFN_vkCmdEndQuery pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndQuery)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndQuery");
    if (pfn) pfn(commandBuffer, queryPool, query);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    static PFN_vkCmdResetQueryPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResetQueryPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResetQueryPool");
    if (pfn) pfn(commandBuffer, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    static PFN_vkCmdWriteTimestamp pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteTimestamp)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteTimestamp");
    if (pfn) pfn(commandBuffer, pipelineStage, queryPool, query);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    static PFN_vkCmdCopyQueryPoolResults pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyQueryPoolResults)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyQueryPoolResults");
    if (pfn) pfn(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    static PFN_vkCmdExecuteCommands pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdExecuteCommands)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdExecuteCommands");
    if (pfn) pfn(commandBuffer, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    static PFN_vkCreateEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateEvent");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pEvent) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyEvent");
    if (pfn) pfn(device, event, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(VkDevice device, VkEvent event) {
    static PFN_vkGetEventStatus pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetEventStatus)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetEventStatus");
    return pfn ? pfn(device, event) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(VkDevice device, VkEvent event) {
    static PFN_vkSetEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetEvent");
    return pfn ? pfn(device, event) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(VkDevice device, VkEvent event) {
    static PFN_vkResetEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetEvent");
    return pfn ? pfn(device, event) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    static PFN_vkCreateBufferView pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateBufferView)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateBufferView");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pView) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyBufferView pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyBufferView)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyBufferView");
    if (pfn) pfn(device, bufferView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    static PFN_vkCreateShaderModule pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateShaderModule)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateShaderModule");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pShaderModule) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyShaderModule pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyShaderModule)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyShaderModule");
    if (pfn) pfn(device, shaderModule, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    static PFN_vkCreatePipelineCache pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreatePipelineCache)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreatePipelineCache");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pPipelineCache) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPipelineCache pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPipelineCache)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPipelineCache");
    if (pfn) pfn(device, pipelineCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    static PFN_vkGetPipelineCacheData pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineCacheData)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineCacheData");
    return pfn ? pfn(device, pipelineCache, pDataSize, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    static PFN_vkMergePipelineCaches pfn = NULL;
    if (!pfn) pfn = (PFN_vkMergePipelineCaches)vk_icdGetInstanceProcAddr(s_vkInstance, "vkMergePipelineCaches");
    return pfn ? pfn(device, dstCache, srcCacheCount, pSrcCaches) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    static PFN_vkCreateComputePipelines pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateComputePipelines)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateComputePipelines");
    return pfn ? pfn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPipeline pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPipeline)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPipeline");
    if (pfn) pfn(device, pipeline, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    static PFN_vkCreatePipelineLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreatePipelineLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreatePipelineLayout");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pPipelineLayout) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPipelineLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPipelineLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPipelineLayout");
    if (pfn) pfn(device, pipelineLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    static PFN_vkCreateSampler pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSampler)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSampler");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSampler) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySampler pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySampler)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySampler");
    if (pfn) pfn(device, sampler, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    static PFN_vkCreateDescriptorSetLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDescriptorSetLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDescriptorSetLayout");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSetLayout) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDescriptorSetLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDescriptorSetLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDescriptorSetLayout");
    if (pfn) pfn(device, descriptorSetLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    static PFN_vkCreateDescriptorPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDescriptorPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDescriptorPool");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pDescriptorPool) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDescriptorPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDescriptorPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDescriptorPool");
    if (pfn) pfn(device, descriptorPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    static PFN_vkResetDescriptorPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetDescriptorPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetDescriptorPool");
    return pfn ? pfn(device, descriptorPool, flags) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
    static PFN_vkAllocateDescriptorSets pfn = NULL;
    if (!pfn) pfn = (PFN_vkAllocateDescriptorSets)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAllocateDescriptorSets");
    return pfn ? pfn(device, pAllocateInfo, pDescriptorSets) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
    static PFN_vkFreeDescriptorSets pfn = NULL;
    if (!pfn) pfn = (PFN_vkFreeDescriptorSets)vk_icdGetInstanceProcAddr(s_vkInstance, "vkFreeDescriptorSets");
    return pfn ? pfn(device, descriptorPool, descriptorSetCount, pDescriptorSets) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    static PFN_vkUpdateDescriptorSets pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateDescriptorSets)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateDescriptorSets");
    if (pfn) pfn(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    static PFN_vkCmdBindPipeline pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindPipeline)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindPipeline");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, pipeline);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    static PFN_vkCmdBindDescriptorSets pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorSets)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorSets");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    static PFN_vkCmdClearColorImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdClearColorImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdClearColorImage");
    if (pfn) pfn(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    static PFN_vkCmdDispatch pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatch)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatch");
    if (pfn) pfn(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    static PFN_vkCmdDispatchIndirect pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchIndirect)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchIndirect");
    if (pfn) pfn(commandBuffer, buffer, offset);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    static PFN_vkCmdSetEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetEvent");
    if (pfn) pfn(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    static PFN_vkCmdResetEvent pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResetEvent)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResetEvent");
    if (pfn) pfn(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    static PFN_vkCmdWaitEvents pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWaitEvents)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWaitEvents");
    if (pfn) pfn(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    static PFN_vkCmdPushConstants pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushConstants)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushConstants");
    if (pfn) pfn(commandBuffer, layout, stageFlags, offset, size, pValues);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    static PFN_vkCreateGraphicsPipelines pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateGraphicsPipelines)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateGraphicsPipelines");
    return pfn ? pfn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    static PFN_vkCreateFramebuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateFramebuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateFramebuffer");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pFramebuffer) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyFramebuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyFramebuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyFramebuffer");
    if (pfn) pfn(device, framebuffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    static PFN_vkCreateRenderPass pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateRenderPass)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateRenderPass");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pRenderPass) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyRenderPass pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyRenderPass)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyRenderPass");
    if (pfn) pfn(device, renderPass, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    static PFN_vkGetRenderAreaGranularity pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRenderAreaGranularity)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRenderAreaGranularity");
    if (pfn) pfn(device, renderPass, pGranularity);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    static PFN_vkCmdSetViewport pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewport)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewport");
    if (pfn) pfn(commandBuffer, firstViewport, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    static PFN_vkCmdSetScissor pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetScissor)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetScissor");
    if (pfn) pfn(commandBuffer, firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    static PFN_vkCmdSetLineWidth pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineWidth)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineWidth");
    if (pfn) pfn(commandBuffer, lineWidth);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    static PFN_vkCmdSetDepthBias pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBias)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBias");
    if (pfn) pfn(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    static PFN_vkCmdSetBlendConstants pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetBlendConstants)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetBlendConstants");
    if (pfn) pfn(commandBuffer, blendConstants);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    static PFN_vkCmdSetDepthBounds pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBounds)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBounds");
    if (pfn) pfn(commandBuffer, minDepthBounds, maxDepthBounds);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    static PFN_vkCmdSetStencilCompareMask pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilCompareMask)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilCompareMask");
    if (pfn) pfn(commandBuffer, faceMask, compareMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    static PFN_vkCmdSetStencilWriteMask pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilWriteMask)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilWriteMask");
    if (pfn) pfn(commandBuffer, faceMask, writeMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    static PFN_vkCmdSetStencilReference pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilReference)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilReference");
    if (pfn) pfn(commandBuffer, faceMask, reference);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    static PFN_vkCmdBindIndexBuffer pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindIndexBuffer)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindIndexBuffer");
    if (pfn) pfn(commandBuffer, buffer, offset, indexType);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    static PFN_vkCmdBindVertexBuffers pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindVertexBuffers)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindVertexBuffers");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    static PFN_vkCmdDraw pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDraw)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDraw");
    if (pfn) pfn(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    static PFN_vkCmdDrawIndexed pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexed)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexed");
    if (pfn) pfn(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndirect pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirect)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirect");
    if (pfn) pfn(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndexedIndirect pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirect)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirect");
    if (pfn) pfn(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    static PFN_vkCmdBlitImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBlitImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBlitImage");
    if (pfn) pfn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    static PFN_vkCmdClearDepthStencilImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdClearDepthStencilImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdClearDepthStencilImage");
    if (pfn) pfn(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    static PFN_vkCmdClearAttachments pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdClearAttachments)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdClearAttachments");
    if (pfn) pfn(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    static PFN_vkCmdResolveImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResolveImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResolveImage");
    if (pfn) pfn(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    static PFN_vkCmdBeginRenderPass pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginRenderPass)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginRenderPass");
    if (pfn) pfn(commandBuffer, pRenderPassBegin, contents);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    static PFN_vkCmdNextSubpass pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdNextSubpass)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdNextSubpass");
    if (pfn) pfn(commandBuffer, contents);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndRenderPass pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRenderPass)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRenderPass");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(uint32_t* pApiVersion) {
    static PFN_vkEnumerateInstanceVersion pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumerateInstanceVersion)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumerateInstanceVersion");
    return pfn ? pfn(pApiVersion) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    static PFN_vkBindBufferMemory2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindBufferMemory2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindBufferMemory2");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    static PFN_vkBindImageMemory2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindImageMemory2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindImageMemory2");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    static PFN_vkGetDeviceGroupPeerMemoryFeatures pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceGroupPeerMemoryFeatures)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceGroupPeerMemoryFeatures");
    if (pfn) pfn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    static PFN_vkCmdSetDeviceMask pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDeviceMask)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDeviceMask");
    if (pfn) pfn(commandBuffer, deviceMask);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    static PFN_vkEnumeratePhysicalDeviceGroups pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDeviceGroups)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDeviceGroups");
    return pfn ? pfn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetImageMemoryRequirements2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageMemoryRequirements2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageMemoryRequirements2");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetBufferMemoryRequirements2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferMemoryRequirements2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferMemoryRequirements2");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    static PFN_vkGetImageSparseMemoryRequirements2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSparseMemoryRequirements2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSparseMemoryRequirements2");
    if (pfn) pfn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    static PFN_vkGetPhysicalDeviceFeatures2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFeatures2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFeatures2");
    if (pfn) pfn(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    static PFN_vkGetPhysicalDeviceProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceProperties2");
    if (pfn) pfn(physicalDevice, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    static PFN_vkGetPhysicalDeviceFormatProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFormatProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFormatProperties2");
    if (pfn) pfn(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceImageFormatProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceImageFormatProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceImageFormatProperties2");
    return pfn ? pfn(physicalDevice, pImageFormatInfo, pImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    if (pfn) pfn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    static PFN_vkGetPhysicalDeviceMemoryProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceMemoryProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceMemoryProperties2");
    if (pfn) pfn(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    if (pfn) pfn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    static PFN_vkTrimCommandPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkTrimCommandPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkTrimCommandPool");
    if (pfn) pfn(device, commandPool, flags);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    static PFN_vkGetDeviceQueue2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceQueue2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceQueue2");
    if (pfn) pfn(device, pQueueInfo, pQueue);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    static PFN_vkGetPhysicalDeviceExternalBufferProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalBufferProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalBufferProperties");
    if (pfn) pfn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    static PFN_vkGetPhysicalDeviceExternalFenceProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalFenceProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalFenceProperties");
    if (pfn) pfn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    static PFN_vkGetPhysicalDeviceExternalSemaphoreProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    if (pfn) pfn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    static PFN_vkCmdDispatchBase pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchBase)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchBase");
    if (pfn) pfn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    static PFN_vkCreateDescriptorUpdateTemplate pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDescriptorUpdateTemplate)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDescriptorUpdateTemplate");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDescriptorUpdateTemplate pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDescriptorUpdateTemplate)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDescriptorUpdateTemplate");
    if (pfn) pfn(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    static PFN_vkUpdateDescriptorSetWithTemplate pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateDescriptorSetWithTemplate)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateDescriptorSetWithTemplate");
    if (pfn) pfn(device, descriptorSet, descriptorUpdateTemplate, pData);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    static PFN_vkGetDescriptorSetLayoutSupport pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetLayoutSupport)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetLayoutSupport");
    if (pfn) pfn(device, pCreateInfo, pSupport);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    static PFN_vkCreateSamplerYcbcrConversion pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSamplerYcbcrConversion)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSamplerYcbcrConversion");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pYcbcrConversion) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySamplerYcbcrConversion pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySamplerYcbcrConversion)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySamplerYcbcrConversion");
    if (pfn) pfn(device, ycbcrConversion, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    static PFN_vkResetQueryPool pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetQueryPool)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetQueryPool");
    if (pfn) pfn(device, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    static PFN_vkGetSemaphoreCounterValue pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSemaphoreCounterValue)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSemaphoreCounterValue");
    return pfn ? pfn(device, semaphore, pValue) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    static PFN_vkWaitSemaphores pfn = NULL;
    if (!pfn) pfn = (PFN_vkWaitSemaphores)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWaitSemaphores");
    return pfn ? pfn(device, pWaitInfo, timeout) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    static PFN_vkSignalSemaphore pfn = NULL;
    if (!pfn) pfn = (PFN_vkSignalSemaphore)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSignalSemaphore");
    return pfn ? pfn(device, pSignalInfo) : (VkResult)0;
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    static PFN_vkGetBufferDeviceAddress pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferDeviceAddress)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferDeviceAddress");
    return pfn ? pfn(device, pInfo) : (VkDeviceAddress)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    static PFN_vkGetBufferOpaqueCaptureAddress pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferOpaqueCaptureAddress)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferOpaqueCaptureAddress");
    return pfn ? pfn(device, pInfo) : (uint64_t)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    static PFN_vkGetDeviceMemoryOpaqueCaptureAddress pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceMemoryOpaqueCaptureAddress");
    return pfn ? pfn(device, pInfo) : (uint64_t)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndirectCount pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectCount)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectCount");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndexedIndirectCount pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirectCount)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirectCount");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    static PFN_vkCreateRenderPass2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateRenderPass2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateRenderPass2");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pRenderPass) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo) {
    static PFN_vkCmdBeginRenderPass2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginRenderPass2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginRenderPass2");
    if (pfn) pfn(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo, const VkSubpassEndInfo* pSubpassEndInfo) {
    static PFN_vkCmdNextSubpass2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdNextSubpass2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdNextSubpass2");
    if (pfn) pfn(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    static PFN_vkCmdEndRenderPass2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRenderPass2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRenderPass2");
    if (pfn) pfn(commandBuffer, pSubpassEndInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolProperties* pToolProperties) {
    static PFN_vkGetPhysicalDeviceToolProperties pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceToolProperties)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceToolProperties");
    return pfn ? pfn(physicalDevice, pToolCount, pToolProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    static PFN_vkCreatePrivateDataSlot pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreatePrivateDataSlot)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreatePrivateDataSlot");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pPrivateDataSlot) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPrivateDataSlot(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPrivateDataSlot pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPrivateDataSlot)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPrivateDataSlot");
    if (pfn) pfn(device, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data) {
    static PFN_vkSetPrivateData pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetPrivateData)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetPrivateData");
    return pfn ? pfn(device, objectType, objectHandle, privateDataSlot, data) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPrivateData(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    static PFN_vkGetPrivateData pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPrivateData)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPrivateData");
    if (pfn) pfn(device, objectType, objectHandle, privateDataSlot, pData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    static PFN_vkCmdPipelineBarrier2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPipelineBarrier2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPipelineBarrier2");
    if (pfn) pfn(commandBuffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query) {
    static PFN_vkCmdWriteTimestamp2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteTimestamp2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteTimestamp2");
    if (pfn) pfn(commandBuffer, stage, queryPool, query);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    static PFN_vkQueueSubmit2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueSubmit2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueSubmit2");
    return pfn ? pfn(queue, submitCount, pSubmits, fence) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    static PFN_vkCmdCopyBuffer2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBuffer2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBuffer2");
    if (pfn) pfn(commandBuffer, pCopyBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    static PFN_vkCmdCopyImage2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImage2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImage2");
    if (pfn) pfn(commandBuffer, pCopyImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    static PFN_vkCmdCopyBufferToImage2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBufferToImage2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBufferToImage2");
    if (pfn) pfn(commandBuffer, pCopyBufferToImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    static PFN_vkCmdCopyImageToBuffer2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImageToBuffer2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImageToBuffer2");
    if (pfn) pfn(commandBuffer, pCopyImageToBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceBufferMemoryRequirements(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDeviceBufferMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceBufferMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceBufferMemoryRequirements");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDeviceImageMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageMemoryRequirements");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    static PFN_vkGetDeviceImageSparseMemoryRequirements pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageSparseMemoryRequirements)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageSparseMemoryRequirements");
    if (pfn) pfn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    static PFN_vkCmdSetEvent2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetEvent2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetEvent2");
    if (pfn) pfn(commandBuffer, event, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    static PFN_vkCmdResetEvent2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResetEvent2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResetEvent2");
    if (pfn) pfn(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos) {
    static PFN_vkCmdWaitEvents2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWaitEvents2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWaitEvents2");
    if (pfn) pfn(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    static PFN_vkCmdBlitImage2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBlitImage2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBlitImage2");
    if (pfn) pfn(commandBuffer, pBlitImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    static PFN_vkCmdResolveImage2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResolveImage2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResolveImage2");
    if (pfn) pfn(commandBuffer, pResolveImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    static PFN_vkCmdBeginRendering pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginRendering)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginRendering");
    if (pfn) pfn(commandBuffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRendering(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndRendering pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRendering)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRendering");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    static PFN_vkCmdSetCullMode pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCullMode)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCullMode");
    if (pfn) pfn(commandBuffer, cullMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    static PFN_vkCmdSetFrontFace pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetFrontFace)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetFrontFace");
    if (pfn) pfn(commandBuffer, frontFace);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    static PFN_vkCmdSetPrimitiveTopology pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPrimitiveTopology)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPrimitiveTopology");
    if (pfn) pfn(commandBuffer, primitiveTopology);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    static PFN_vkCmdSetViewportWithCount pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportWithCount)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportWithCount");
    if (pfn) pfn(commandBuffer, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    static PFN_vkCmdSetScissorWithCount pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetScissorWithCount)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetScissorWithCount");
    if (pfn) pfn(commandBuffer, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    static PFN_vkCmdBindVertexBuffers2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindVertexBuffers2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindVertexBuffers2");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    static PFN_vkCmdSetDepthTestEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthTestEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthTestEnable");
    if (pfn) pfn(commandBuffer, depthTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    static PFN_vkCmdSetDepthWriteEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthWriteEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthWriteEnable");
    if (pfn) pfn(commandBuffer, depthWriteEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    static PFN_vkCmdSetDepthCompareOp pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthCompareOp)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthCompareOp");
    if (pfn) pfn(commandBuffer, depthCompareOp);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    static PFN_vkCmdSetDepthBoundsTestEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBoundsTestEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBoundsTestEnable");
    if (pfn) pfn(commandBuffer, depthBoundsTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    static PFN_vkCmdSetStencilTestEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilTestEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilTestEnable");
    if (pfn) pfn(commandBuffer, stencilTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    static PFN_vkCmdSetStencilOp pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilOp)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilOp");
    if (pfn) pfn(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    static PFN_vkCmdSetRasterizerDiscardEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRasterizerDiscardEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRasterizerDiscardEnable");
    if (pfn) pfn(commandBuffer, rasterizerDiscardEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    static PFN_vkCmdSetDepthBiasEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBiasEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBiasEnable");
    if (pfn) pfn(commandBuffer, depthBiasEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    static PFN_vkCmdSetPrimitiveRestartEnable pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPrimitiveRestartEnable)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPrimitiveRestartEnable");
    if (pfn) pfn(commandBuffer, primitiveRestartEnable);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    static PFN_vkMapMemory2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkMapMemory2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkMapMemory2");
    return pfn ? pfn(device, pMemoryMapInfo, ppData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    static PFN_vkUnmapMemory2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkUnmapMemory2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUnmapMemory2");
    return pfn ? pfn(device, pMemoryUnmapInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo, VkSubresourceLayout2* pLayout) {
    static PFN_vkGetDeviceImageSubresourceLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageSubresourceLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageSubresourceLayout");
    if (pfn) pfn(device, pInfo, pLayout);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout2(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource, VkSubresourceLayout2* pLayout) {
    static PFN_vkGetImageSubresourceLayout2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSubresourceLayout2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSubresourceLayout2");
    if (pfn) pfn(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    static PFN_vkCopyMemoryToImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMemoryToImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMemoryToImage");
    return pfn ? pfn(device, pCopyMemoryToImageInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    static PFN_vkCopyImageToMemory pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyImageToMemory)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyImageToMemory");
    return pfn ? pfn(device, pCopyImageToMemoryInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    static PFN_vkCopyImageToImage pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyImageToImage)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyImageToImage");
    return pfn ? pfn(device, pCopyImageToImageInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkTransitionImageLayout(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo* pTransitions) {
    static PFN_vkTransitionImageLayout pfn = NULL;
    if (!pfn) pfn = (PFN_vkTransitionImageLayout)vk_icdGetInstanceProcAddr(s_vkInstance, "vkTransitionImageLayout");
    return pfn ? pfn(device, transitionCount, pTransitions) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    static PFN_vkCmdPushDescriptorSet pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSet)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSet");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    static PFN_vkCmdPushDescriptorSetWithTemplate pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSetWithTemplate)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSetWithTemplate");
    if (pfn) pfn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    static PFN_vkCmdBindDescriptorSets2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorSets2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorSets2");
    if (pfn) pfn(commandBuffer, pBindDescriptorSetsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    static PFN_vkCmdPushConstants2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushConstants2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushConstants2");
    if (pfn) pfn(commandBuffer, pPushConstantsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    static PFN_vkCmdPushDescriptorSet2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSet2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSet2");
    if (pfn) pfn(commandBuffer, pPushDescriptorSetInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo) {
    static PFN_vkCmdPushDescriptorSetWithTemplate2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSetWithTemplate2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSetWithTemplate2");
    if (pfn) pfn(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    static PFN_vkCmdSetLineStipple pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineStipple)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineStipple");
    if (pfn) pfn(commandBuffer, lineStippleFactor, lineStipplePattern);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType) {
    static PFN_vkCmdBindIndexBuffer2 pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindIndexBuffer2)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindIndexBuffer2");
    if (pfn) pfn(commandBuffer, buffer, offset, size, indexType);
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderingAreaGranularity(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo, VkExtent2D* pGranularity) {
    static PFN_vkGetRenderingAreaGranularity pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRenderingAreaGranularity)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRenderingAreaGranularity");
    if (pfn) pfn(device, pRenderingAreaInfo, pGranularity);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    static PFN_vkCmdSetRenderingAttachmentLocations pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRenderingAttachmentLocations)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRenderingAttachmentLocations");
    if (pfn) pfn(commandBuffer, pLocationInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    static PFN_vkCmdSetRenderingInputAttachmentIndices pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRenderingInputAttachmentIndices)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRenderingInputAttachmentIndices");
    if (pfn) pfn(commandBuffer, pInputAttachmentIndexInfo);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySurfaceKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySurfaceKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySurfaceKHR");
    if (pfn) pfn(instance, surface, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
    static PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, surface, pSupported) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    return pfn ? pfn(physicalDevice, surface, pSurfaceCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    return pfn ? pfn(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    return pfn ? pfn(physicalDevice, surface, pPresentModeCount, pPresentModes) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    static PFN_vkCreateSwapchainKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSwapchainKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSwapchainKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSwapchain) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySwapchainKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySwapchainKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySwapchainKHR");
    if (pfn) pfn(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    static PFN_vkGetSwapchainImagesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSwapchainImagesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSwapchainImagesKHR");
    return pfn ? pfn(device, swapchain, pSwapchainImageCount, pSwapchainImages) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    static PFN_vkAcquireNextImageKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkAcquireNextImageKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAcquireNextImageKHR");
    return pfn ? pfn(device, swapchain, timeout, semaphore, fence, pImageIndex) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    static PFN_vkQueuePresentKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueuePresentKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueuePresentKHR");
    return pfn ? pfn(queue, pPresentInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    static PFN_vkGetDeviceGroupPresentCapabilitiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceGroupPresentCapabilitiesKHR");
    return pfn ? pfn(device, pDeviceGroupPresentCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    static PFN_vkGetDeviceGroupSurfacePresentModesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceGroupSurfacePresentModesKHR");
    return pfn ? pfn(device, surface, pModes) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) {
    static PFN_vkGetPhysicalDevicePresentRectanglesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDevicePresentRectanglesKHR");
    return pfn ? pfn(physicalDevice, surface, pRectCount, pRects) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    static PFN_vkAcquireNextImage2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkAcquireNextImage2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAcquireNextImage2KHR");
    return pfn ? pfn(device, pAcquireInfo, pImageIndex) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) {
    static PFN_vkGetPhysicalDeviceDisplayPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) {
    static PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) {
    static PFN_vkGetDisplayPlaneSupportedDisplaysKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    return pfn ? pfn(physicalDevice, planeIndex, pDisplayCount, pDisplays) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) {
    static PFN_vkGetDisplayModePropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDisplayModePropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDisplayModePropertiesKHR");
    return pfn ? pfn(physicalDevice, display, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) {
    static PFN_vkCreateDisplayModeKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDisplayModeKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDisplayModeKHR");
    return pfn ? pfn(physicalDevice, display, pCreateInfo, pAllocator, pMode) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) {
    static PFN_vkGetDisplayPlaneCapabilitiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDisplayPlaneCapabilitiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDisplayPlaneCapabilitiesKHR");
    return pfn ? pfn(physicalDevice, mode, planeIndex, pCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    static PFN_vkCreateDisplayPlaneSurfaceKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDisplayPlaneSurfaceKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDisplayPlaneSurfaceKHR");
    return pfn ? pfn(instance, pCreateInfo, pAllocator, pSurface) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    static PFN_vkCreateSharedSwapchainsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSharedSwapchainsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSharedSwapchainsKHR");
    return pfn ? pfn(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR* pVideoProfile, VkVideoCapabilitiesKHR* pCapabilities) {
    static PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    return pfn ? pfn(physicalDevice, pVideoProfile, pCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo, uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties) {
    static PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
    return pfn ? pfn(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) {
    static PFN_vkCreateVideoSessionKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateVideoSessionKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateVideoSessionKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pVideoSession) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyVideoSessionKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyVideoSessionKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyVideoSessionKHR");
    if (pfn) pfn(device, videoSession, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pMemoryRequirementsCount, VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements) {
    static PFN_vkGetVideoSessionMemoryRequirementsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetVideoSessionMemoryRequirementsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetVideoSessionMemoryRequirementsKHR");
    return pfn ? pfn(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos) {
    static PFN_vkBindVideoSessionMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindVideoSessionMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindVideoSessionMemoryKHR");
    return pfn ? pfn(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionParametersKHR* pVideoSessionParameters) {
    static PFN_vkCreateVideoSessionParametersKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateVideoSessionParametersKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateVideoSessionParametersKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pVideoSessionParameters) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    static PFN_vkUpdateVideoSessionParametersKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateVideoSessionParametersKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateVideoSessionParametersKHR");
    return pfn ? pfn(device, videoSessionParameters, pUpdateInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyVideoSessionParametersKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyVideoSessionParametersKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyVideoSessionParametersKHR");
    if (pfn) pfn(device, videoSessionParameters, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) {
    static PFN_vkCmdBeginVideoCodingKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginVideoCodingKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginVideoCodingKHR");
    if (pfn) pfn(commandBuffer, pBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) {
    static PFN_vkCmdEndVideoCodingKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndVideoCodingKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndVideoCodingKHR");
    if (pfn) pfn(commandBuffer, pEndCodingInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo) {
    static PFN_vkCmdControlVideoCodingKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdControlVideoCodingKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdControlVideoCodingKHR");
    if (pfn) pfn(commandBuffer, pCodingControlInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo) {
    static PFN_vkCmdDecodeVideoKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDecodeVideoKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDecodeVideoKHR");
    if (pfn) pfn(commandBuffer, pDecodeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo) {
    static PFN_vkCmdBeginRenderingKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginRenderingKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginRenderingKHR");
    if (pfn) pfn(commandBuffer, pRenderingInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndRenderingKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRenderingKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRenderingKHR");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    static PFN_vkGetPhysicalDeviceFeatures2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFeatures2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFeatures2KHR");
    if (pfn) pfn(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    static PFN_vkGetPhysicalDeviceProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceProperties2KHR");
    if (pfn) pfn(physicalDevice, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    static PFN_vkGetPhysicalDeviceFormatProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFormatProperties2KHR");
    if (pfn) pfn(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceImageFormatProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    return pfn ? pfn(physicalDevice, pImageFormatInfo, pImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    if (pfn) pfn(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    static PFN_vkGetPhysicalDeviceMemoryProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    if (pfn) pfn(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
    if (pfn) pfn(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeaturesKHR(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    static PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    if (pfn) pfn(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    static PFN_vkCmdSetDeviceMaskKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDeviceMaskKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDeviceMaskKHR");
    if (pfn) pfn(commandBuffer, deviceMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    static PFN_vkCmdDispatchBaseKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchBaseKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchBaseKHR");
    if (pfn) pfn(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL vkTrimCommandPoolKHR(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    static PFN_vkTrimCommandPoolKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkTrimCommandPoolKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkTrimCommandPoolKHR");
    if (pfn) pfn(device, commandPool, flags);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    static PFN_vkEnumeratePhysicalDeviceGroupsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDeviceGroupsKHR");
    return pfn ? pfn(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    static PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
    if (pfn) pfn(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) {
    static PFN_vkGetMemoryFdKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetMemoryFdKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetMemoryFdKHR");
    return pfn ? pfn(device, pGetFdInfo, pFd) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) {
    static PFN_vkGetMemoryFdPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetMemoryFdPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetMemoryFdPropertiesKHR");
    return pfn ? pfn(device, handleType, fd, pMemoryFdProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    static PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    if (pfn) pfn(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) {
    static PFN_vkImportSemaphoreFdKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkImportSemaphoreFdKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkImportSemaphoreFdKHR");
    return pfn ? pfn(device, pImportSemaphoreFdInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) {
    static PFN_vkGetSemaphoreFdKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSemaphoreFdKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSemaphoreFdKHR");
    return pfn ? pfn(device, pGetFdInfo, pFd) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) {
    static PFN_vkCmdPushDescriptorSetKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSetKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSetKHR");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) {
    static PFN_vkCmdPushDescriptorSetWithTemplateKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSetWithTemplateKHR");
    if (pfn) pfn(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    static PFN_vkCreateDescriptorUpdateTemplateKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDescriptorUpdateTemplateKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDescriptorUpdateTemplateKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDescriptorUpdateTemplateKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDescriptorUpdateTemplateKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDescriptorUpdateTemplateKHR");
    if (pfn) pfn(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    static PFN_vkUpdateDescriptorSetWithTemplateKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateDescriptorSetWithTemplateKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateDescriptorSetWithTemplateKHR");
    if (pfn) pfn(device, descriptorSet, descriptorUpdateTemplate, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    static PFN_vkCreateRenderPass2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateRenderPass2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateRenderPass2KHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pRenderPass) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, const VkSubpassBeginInfo* pSubpassBeginInfo) {
    static PFN_vkCmdBeginRenderPass2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginRenderPass2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginRenderPass2KHR");
    if (pfn) pfn(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo, const VkSubpassEndInfo* pSubpassEndInfo) {
    static PFN_vkCmdNextSubpass2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdNextSubpass2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdNextSubpass2KHR");
    if (pfn) pfn(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo) {
    static PFN_vkCmdEndRenderPass2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRenderPass2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRenderPass2KHR");
    if (pfn) pfn(commandBuffer, pSubpassEndInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    static PFN_vkGetSwapchainStatusKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSwapchainStatusKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSwapchainStatusKHR");
    return pfn ? pfn(device, swapchain) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    static PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    if (pfn) pfn(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo) {
    static PFN_vkImportFenceFdKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkImportFenceFdKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkImportFenceFdKHR");
    return pfn ? pfn(device, pImportFenceFdInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) {
    static PFN_vkGetFenceFdKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetFenceFdKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetFenceFdKHR");
    return pfn ? pfn(device, pGetFdInfo, pFd) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) {
    static PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) {
    static PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    if (pfn) pfn(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo) {
    static PFN_vkAcquireProfilingLockKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkAcquireProfilingLockKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAcquireProfilingLockKHR");
    return pfn ? pfn(device, pInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkReleaseProfilingLockKHR(VkDevice device) {
    static PFN_vkReleaseProfilingLockKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleaseProfilingLockKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleaseProfilingLockKHR");
    if (pfn) pfn(device);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) {
    static PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    return pfn ? pfn(physicalDevice, pSurfaceInfo, pSurfaceCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) {
    static PFN_vkGetPhysicalDeviceSurfaceFormats2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
    return pfn ? pfn(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) {
    static PFN_vkGetPhysicalDeviceDisplayProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceDisplayProperties2KHR");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) {
    static PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) {
    static PFN_vkGetDisplayModeProperties2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDisplayModeProperties2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDisplayModeProperties2KHR");
    return pfn ? pfn(physicalDevice, display, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) {
    static PFN_vkGetDisplayPlaneCapabilities2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDisplayPlaneCapabilities2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDisplayPlaneCapabilities2KHR");
    return pfn ? pfn(physicalDevice, pDisplayPlaneInfo, pCapabilities) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetImageMemoryRequirements2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageMemoryRequirements2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageMemoryRequirements2KHR");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2KHR(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetBufferMemoryRequirements2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferMemoryRequirements2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferMemoryRequirements2KHR");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    static PFN_vkGetImageSparseMemoryRequirements2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSparseMemoryRequirements2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSparseMemoryRequirements2KHR");
    if (pfn) pfn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    static PFN_vkCreateSamplerYcbcrConversionKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateSamplerYcbcrConversionKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateSamplerYcbcrConversionKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pYcbcrConversion) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroySamplerYcbcrConversionKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroySamplerYcbcrConversionKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroySamplerYcbcrConversionKHR");
    if (pfn) pfn(device, ycbcrConversion, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    static PFN_vkBindBufferMemory2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindBufferMemory2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindBufferMemory2KHR");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    static PFN_vkBindImageMemory2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindImageMemory2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindImageMemory2KHR");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    static PFN_vkGetDescriptorSetLayoutSupportKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetLayoutSupportKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetLayoutSupportKHR");
    if (pfn) pfn(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndirectCountKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectCountKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectCountKHR");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndexedIndirectCountKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirectCountKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirectCountKHR");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    static PFN_vkGetSemaphoreCounterValueKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSemaphoreCounterValueKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSemaphoreCounterValueKHR");
    return pfn ? pfn(device, semaphore, pValue) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    static PFN_vkWaitSemaphoresKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkWaitSemaphoresKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWaitSemaphoresKHR");
    return pfn ? pfn(device, pWaitInfo, timeout) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    static PFN_vkSignalSemaphoreKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkSignalSemaphoreKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSignalSemaphoreKHR");
    return pfn ? pfn(device, pSignalInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
    static PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
    return pfn ? pfn(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    static PFN_vkCmdSetFragmentShadingRateKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetFragmentShadingRateKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetFragmentShadingRateKHR");
    if (pfn) pfn(commandBuffer, pFragmentSize, combinerOps);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer, const VkRenderingAttachmentLocationInfo* pLocationInfo) {
    static PFN_vkCmdSetRenderingAttachmentLocationsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRenderingAttachmentLocationsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRenderingAttachmentLocationsKHR");
    if (pfn) pfn(commandBuffer, pLocationInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer, const VkRenderingInputAttachmentIndexInfo* pInputAttachmentIndexInfo) {
    static PFN_vkCmdSetRenderingInputAttachmentIndicesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRenderingInputAttachmentIndicesKHR");
    if (pfn) pfn(commandBuffer, pInputAttachmentIndexInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForPresentKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) {
    static PFN_vkWaitForPresentKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkWaitForPresentKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWaitForPresentKHR");
    return pfn ? pfn(device, swapchain, presentId, timeout) : (VkResult)0;
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    static PFN_vkGetBufferDeviceAddressKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferDeviceAddressKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferDeviceAddressKHR");
    return pfn ? pfn(device, pInfo) : (VkDeviceAddress)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    static PFN_vkGetBufferOpaqueCaptureAddressKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferOpaqueCaptureAddressKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferOpaqueCaptureAddressKHR");
    return pfn ? pfn(device, pInfo) : (uint64_t)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    static PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
    return pfn ? pfn(device, pInfo) : (uint64_t)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDeferredOperationKHR(VkDevice device, const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) {
    static PFN_vkCreateDeferredOperationKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDeferredOperationKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDeferredOperationKHR");
    return pfn ? pfn(device, pAllocator, pDeferredOperation) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDeferredOperationKHR(VkDevice device, VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDeferredOperationKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDeferredOperationKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDeferredOperationKHR");
    if (pfn) pfn(device, operation, pAllocator);
}

VKAPI_ATTR uint32_t VKAPI_CALL vkGetDeferredOperationMaxConcurrencyKHR(VkDevice device, VkDeferredOperationKHR operation) {
    static PFN_vkGetDeferredOperationMaxConcurrencyKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeferredOperationMaxConcurrencyKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeferredOperationMaxConcurrencyKHR");
    return pfn ? pfn(device, operation) : (uint32_t)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeferredOperationResultKHR(VkDevice device, VkDeferredOperationKHR operation) {
    static PFN_vkGetDeferredOperationResultKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeferredOperationResultKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeferredOperationResultKHR");
    return pfn ? pfn(device, operation) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeferredOperationJoinKHR(VkDevice device, VkDeferredOperationKHR operation) {
    static PFN_vkDeferredOperationJoinKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDeferredOperationJoinKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDeferredOperationJoinKHR");
    return pfn ? pfn(device, operation) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) {
    static PFN_vkGetPipelineExecutablePropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineExecutablePropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineExecutablePropertiesKHR");
    return pfn ? pfn(device, pPipelineInfo, pExecutableCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineExecutableStatisticsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) {
    static PFN_vkGetPipelineExecutableStatisticsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineExecutableStatisticsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineExecutableStatisticsKHR");
    return pfn ? pfn(device, pExecutableInfo, pStatisticCount, pStatistics) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineExecutableInternalRepresentationsKHR(VkDevice device, const VkPipelineExecutableInfoKHR* pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) {
    static PFN_vkGetPipelineExecutableInternalRepresentationsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineExecutableInternalRepresentationsKHR");
    return pfn ? pfn(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory2KHR(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData) {
    static PFN_vkMapMemory2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkMapMemory2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkMapMemory2KHR");
    return pfn ? pfn(device, pMemoryMapInfo, ppData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo) {
    static PFN_vkUnmapMemory2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkUnmapMemory2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUnmapMemory2KHR");
    return pfn ? pfn(device, pMemoryUnmapInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo, VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties) {
    static PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
    return pfn ? pfn(physicalDevice, pQualityLevelInfo, pQualityLevelProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEncodedVideoSessionParametersKHR(VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR* pVideoSessionParametersInfo, VkVideoEncodeSessionParametersFeedbackInfoKHR* pFeedbackInfo, size_t* pDataSize, void* pData) {
    static PFN_vkGetEncodedVideoSessionParametersKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetEncodedVideoSessionParametersKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetEncodedVideoSessionParametersKHR");
    return pfn ? pfn(device, pVideoSessionParametersInfo, pFeedbackInfo, pDataSize, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) {
    static PFN_vkCmdEncodeVideoKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEncodeVideoKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEncodeVideoKHR");
    if (pfn) pfn(commandBuffer, pEncodeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo) {
    static PFN_vkCmdSetEvent2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetEvent2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetEvent2KHR");
    if (pfn) pfn(commandBuffer, event, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    static PFN_vkCmdResetEvent2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResetEvent2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResetEvent2KHR");
    if (pfn) pfn(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, const VkDependencyInfo* pDependencyInfos) {
    static PFN_vkCmdWaitEvents2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWaitEvents2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWaitEvents2KHR");
    if (pfn) pfn(commandBuffer, eventCount, pEvents, pDependencyInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo) {
    static PFN_vkCmdPipelineBarrier2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPipelineBarrier2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPipelineBarrier2KHR");
    if (pfn) pfn(commandBuffer, pDependencyInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query) {
    static PFN_vkCmdWriteTimestamp2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteTimestamp2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteTimestamp2KHR");
    if (pfn) pfn(commandBuffer, stage, queryPool, query);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence) {
    static PFN_vkQueueSubmit2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueSubmit2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueSubmit2KHR");
    return pfn ? pfn(queue, submitCount, pSubmits, fence) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer3KHR(VkCommandBuffer commandBuffer, const VkBindIndexBuffer3InfoKHR* pInfo) {
    static PFN_vkCmdBindIndexBuffer3KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindIndexBuffer3KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindIndexBuffer3KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers3KHR(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBindVertexBuffer3InfoKHR* pBindingInfos) {
    static PFN_vkCmdBindVertexBuffers3KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindVertexBuffers3KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindVertexBuffers3KHR");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBindingInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    static PFN_vkCmdDrawIndirect2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirect2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirect2KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    static PFN_vkCmdDrawIndexedIndirect2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirect2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirect2KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect2KHR(VkCommandBuffer commandBuffer, const VkDispatchIndirect2InfoKHR* pInfo) {
    static PFN_vkCmdDispatchIndirect2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchIndirect2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchIndirect2KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryInfoKHR* pCopyMemoryInfo) {
    static PFN_vkCmdCopyMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryKHR");
    if (pfn) pfn(commandBuffer, pCopyMemoryInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToImageKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryImageInfoKHR* pCopyMemoryInfo) {
    static PFN_vkCmdCopyMemoryToImageKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryToImageKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryToImageKHR");
    if (pfn) pfn(commandBuffer, pCopyMemoryInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyDeviceMemoryImageInfoKHR* pCopyMemoryInfo) {
    static PFN_vkCmdCopyImageToMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImageToMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImageToMemoryKHR");
    if (pfn) pfn(commandBuffer, pCopyMemoryInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdateMemoryKHR(VkCommandBuffer commandBuffer, const VkDeviceAddressRangeKHR* pDstRange, VkAddressCommandFlagsKHR dstFlags, VkDeviceSize dataSize, const void* pData) {
    static PFN_vkCmdUpdateMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdUpdateMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdUpdateMemoryKHR");
    if (pfn) pfn(commandBuffer, pDstRange, dstFlags, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdFillMemoryKHR(VkCommandBuffer commandBuffer, const VkDeviceAddressRangeKHR* pDstRange, VkAddressCommandFlagsKHR dstFlags, uint32_t data) {
    static PFN_vkCmdFillMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdFillMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdFillMemoryKHR");
    if (pfn) pfn(commandBuffer, pDstRange, dstFlags, data);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResultsToMemoryKHR(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, const VkStridedDeviceAddressRangeKHR* pDstRange, VkAddressCommandFlagsKHR dstFlags, VkQueryResultFlags queryResultFlags) {
    static PFN_vkCmdCopyQueryPoolResultsToMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyQueryPoolResultsToMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyQueryPoolResultsToMemoryKHR");
    if (pfn) pfn(commandBuffer, queryPool, firstQuery, queryCount, pDstRange, dstFlags, queryResultFlags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCount2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    static PFN_vkCmdDrawIndirectCount2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectCount2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectCount2KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCount2KHR(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    static PFN_vkCmdDrawIndexedIndirectCount2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirectCount2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirectCount2KHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginConditionalRendering2EXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfo2EXT* pConditionalRenderingBegin) {
    static PFN_vkCmdBeginConditionalRendering2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginConditionalRendering2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginConditionalRendering2EXT");
    if (pfn) pfn(commandBuffer, pConditionalRenderingBegin);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindTransformFeedbackBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBindTransformFeedbackBuffer2InfoEXT* pBindingInfos) {
    static PFN_vkCmdBindTransformFeedbackBuffers2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindTransformFeedbackBuffers2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindTransformFeedbackBuffers2EXT");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBindingInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginTransformFeedback2EXT(VkCommandBuffer commandBuffer, uint32_t firstCounterRange, uint32_t counterRangeCount, const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfos) {
    static PFN_vkCmdBeginTransformFeedback2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginTransformFeedback2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginTransformFeedback2EXT");
    if (pfn) pfn(commandBuffer, firstCounterRange, counterRangeCount, pCounterInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndTransformFeedback2EXT(VkCommandBuffer commandBuffer, uint32_t firstCounterRange, uint32_t counterRangeCount, const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfos) {
    static PFN_vkCmdEndTransformFeedback2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndTransformFeedback2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndTransformFeedback2EXT");
    if (pfn) pfn(commandBuffer, firstCounterRange, counterRangeCount, pCounterInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectByteCount2EXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, const VkBindTransformFeedbackBuffer2InfoEXT* pCounterInfo, uint32_t counterOffset, uint32_t vertexStride) {
    static PFN_vkCmdDrawIndirectByteCount2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectByteCount2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectByteCount2EXT");
    if (pfn) pfn(commandBuffer, instanceCount, firstInstance, pCounterInfo, counterOffset, vertexStride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirect2EXT(VkCommandBuffer commandBuffer, const VkDrawIndirect2InfoKHR* pInfo) {
    static PFN_vkCmdDrawMeshTasksIndirect2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirect2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirect2EXT");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectCount2EXT(VkCommandBuffer commandBuffer, const VkDrawIndirectCount2InfoKHR* pInfo) {
    static PFN_vkCmdDrawMeshTasksIndirectCount2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirectCount2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirectCount2EXT");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteMarkerToMemoryAMD(VkCommandBuffer commandBuffer, const VkMemoryMarkerInfoAMD* pInfo) {
    static PFN_vkCmdWriteMarkerToMemoryAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteMarkerToMemoryAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteMarkerToMemoryAMD");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructure2KHR(VkDevice device, const VkAccelerationStructureCreateInfo2KHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure) {
    static PFN_vkCreateAccelerationStructure2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateAccelerationStructure2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateAccelerationStructure2KHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pAccelerationStructure) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) {
    static PFN_vkCmdCopyBuffer2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBuffer2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBuffer2KHR");
    if (pfn) pfn(commandBuffer, pCopyBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) {
    static PFN_vkCmdCopyImage2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImage2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImage2KHR");
    if (pfn) pfn(commandBuffer, pCopyImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) {
    static PFN_vkCmdCopyBufferToImage2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyBufferToImage2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyBufferToImage2KHR");
    if (pfn) pfn(commandBuffer, pCopyBufferToImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) {
    static PFN_vkCmdCopyImageToBuffer2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyImageToBuffer2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyImageToBuffer2KHR");
    if (pfn) pfn(commandBuffer, pCopyImageToBufferInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) {
    static PFN_vkCmdBlitImage2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBlitImage2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBlitImage2KHR");
    if (pfn) pfn(commandBuffer, pBlitImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) {
    static PFN_vkCmdResolveImage2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdResolveImage2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdResolveImage2KHR");
    if (pfn) pfn(commandBuffer, pResolveImageInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) {
    static PFN_vkCmdTraceRaysIndirect2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdTraceRaysIndirect2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdTraceRaysIndirect2KHR");
    if (pfn) pfn(commandBuffer, indirectDeviceAddress);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceBufferMemoryRequirementsKHR(VkDevice device, const VkDeviceBufferMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDeviceBufferMemoryRequirementsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceBufferMemoryRequirementsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceBufferMemoryRequirementsKHR");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDeviceImageMemoryRequirementsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageMemoryRequirementsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageMemoryRequirementsKHR");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSparseMemoryRequirementsKHR(VkDevice device, const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    static PFN_vkGetDeviceImageSparseMemoryRequirementsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageSparseMemoryRequirementsKHR");
    if (pfn) pfn(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType) {
    static PFN_vkCmdBindIndexBuffer2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindIndexBuffer2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindIndexBuffer2KHR");
    if (pfn) pfn(commandBuffer, buffer, offset, size, indexType);
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderingAreaGranularityKHR(VkDevice device, const VkRenderingAreaInfo* pRenderingAreaInfo, VkExtent2D* pGranularity) {
    static PFN_vkGetRenderingAreaGranularityKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRenderingAreaGranularityKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRenderingAreaGranularityKHR");
    if (pfn) pfn(device, pRenderingAreaInfo, pGranularity);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceImageSubresourceLayoutKHR(VkDevice device, const VkDeviceImageSubresourceInfo* pInfo, VkSubresourceLayout2* pLayout) {
    static PFN_vkGetDeviceImageSubresourceLayoutKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceImageSubresourceLayoutKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceImageSubresourceLayoutKHR");
    if (pfn) pfn(device, pInfo, pLayout);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout2KHR(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource, VkSubresourceLayout2* pLayout) {
    static PFN_vkGetImageSubresourceLayout2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSubresourceLayout2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSubresourceLayout2KHR");
    if (pfn) pfn(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForPresent2KHR(VkDevice device, VkSwapchainKHR swapchain, const VkPresentWait2InfoKHR* pPresentWait2Info) {
    static PFN_vkWaitForPresent2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkWaitForPresent2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWaitForPresent2KHR");
    return pfn ? pfn(device, swapchain, pPresentWait2Info) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineBinariesKHR(VkDevice device, const VkPipelineBinaryCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineBinaryHandlesInfoKHR* pBinaries) {
    static PFN_vkCreatePipelineBinariesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreatePipelineBinariesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreatePipelineBinariesKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pBinaries) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineBinaryKHR(VkDevice device, VkPipelineBinaryKHR pipelineBinary, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPipelineBinaryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPipelineBinaryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPipelineBinaryKHR");
    if (pfn) pfn(device, pipelineBinary, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineKeyKHR(VkDevice device, const VkPipelineCreateInfoKHR* pPipelineCreateInfo, VkPipelineBinaryKeyKHR* pPipelineKey) {
    static PFN_vkGetPipelineKeyKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineKeyKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineKeyKHR");
    return pfn ? pfn(device, pPipelineCreateInfo, pPipelineKey) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineBinaryDataKHR(VkDevice device, const VkPipelineBinaryDataInfoKHR* pInfo, VkPipelineBinaryKeyKHR* pPipelineBinaryKey, size_t* pPipelineBinaryDataSize, void* pPipelineBinaryData) {
    static PFN_vkGetPipelineBinaryDataKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineBinaryDataKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineBinaryDataKHR");
    return pfn ? pfn(device, pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize, pPipelineBinaryData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkReleaseCapturedPipelineDataKHR(VkDevice device, const VkReleaseCapturedPipelineDataInfoKHR* pInfo, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkReleaseCapturedPipelineDataKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleaseCapturedPipelineDataKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleaseCapturedPipelineDataKHR");
    return pfn ? pfn(device, pInfo, pAllocator) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkReleaseSwapchainImagesKHR(VkDevice device, const VkReleaseSwapchainImagesInfoKHR* pReleaseInfo) {
    static PFN_vkReleaseSwapchainImagesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleaseSwapchainImagesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleaseSwapchainImagesKHR");
    return pfn ? pfn(device, pReleaseInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesKHR* pProperties) {
    static PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    static PFN_vkCmdSetLineStippleKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineStippleKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineStippleKHR");
    if (pfn) pfn(commandBuffer, lineStippleFactor, lineStipplePattern);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainKHR* pTimeDomains) {
    static PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR");
    return pfn ? pfn(physicalDevice, pTimeDomainCount, pTimeDomains) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetCalibratedTimestampsKHR(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    static PFN_vkGetCalibratedTimestampsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetCalibratedTimestampsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetCalibratedTimestampsKHR");
    return pfn ? pfn(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo) {
    static PFN_vkCmdBindDescriptorSets2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorSets2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorSets2KHR");
    if (pfn) pfn(commandBuffer, pBindDescriptorSetsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo) {
    static PFN_vkCmdPushConstants2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushConstants2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushConstants2KHR");
    if (pfn) pfn(commandBuffer, pPushConstantsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo) {
    static PFN_vkCmdPushDescriptorSet2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSet2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSet2KHR");
    if (pfn) pfn(commandBuffer, pPushDescriptorSetInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplate2KHR(VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo* pPushDescriptorSetWithTemplateInfo) {
    static PFN_vkCmdPushDescriptorSetWithTemplate2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDescriptorSetWithTemplate2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDescriptorSetWithTemplate2KHR");
    if (pfn) pfn(commandBuffer, pPushDescriptorSetWithTemplateInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo) {
    static PFN_vkCmdSetDescriptorBufferOffsets2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDescriptorBufferOffsets2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDescriptorBufferOffsets2EXT");
    if (pfn) pfn(commandBuffer, pSetDescriptorBufferOffsetsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorBufferEmbeddedSamplers2EXT(VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* pBindDescriptorBufferEmbeddedSamplersInfo) {
    static PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");
    if (pfn) pfn(commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryIndirectKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryIndirectInfoKHR* pCopyMemoryIndirectInfo) {
    static PFN_vkCmdCopyMemoryIndirectKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryIndirectKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryIndirectKHR");
    if (pfn) pfn(commandBuffer, pCopyMemoryIndirectInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToImageIndirectKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToImageIndirectInfoKHR* pCopyMemoryToImageIndirectInfo) {
    static PFN_vkCmdCopyMemoryToImageIndirectKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryToImageIndirectKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryToImageIndirectKHR");
    if (pfn) pfn(commandBuffer, pCopyMemoryToImageIndirectInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceFaultReportsKHR(VkDevice device, uint64_t timeout, uint32_t* pFaultCounts, VkDeviceFaultInfoKHR* pFaultInfo) {
    static PFN_vkGetDeviceFaultReportsKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceFaultReportsKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceFaultReportsKHR");
    return pfn ? pfn(device, timeout, pFaultCounts, pFaultInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceFaultDebugInfoKHR(VkDevice device, VkDeviceFaultDebugInfoKHR* pDebugInfo) {
    static PFN_vkGetDeviceFaultDebugInfoKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceFaultDebugInfoKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceFaultDebugInfoKHR");
    return pfn ? pfn(device, pDebugInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRendering2KHR(VkCommandBuffer commandBuffer, const VkRenderingEndInfoKHR* pRenderingEndInfo) {
    static PFN_vkCmdEndRendering2KHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRendering2KHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRendering2KHR");
    if (pfn) pfn(commandBuffer, pRenderingEndInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    static PFN_vkCreateDebugReportCallbackEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDebugReportCallbackEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDebugReportCallbackEXT");
    return pfn ? pfn(instance, pCreateInfo, pAllocator, pCallback) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDebugReportCallbackEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDebugReportCallbackEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDebugReportCallbackEXT");
    if (pfn) pfn(instance, callback, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
    static PFN_vkDebugReportMessageEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDebugReportMessageEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDebugReportMessageEXT");
    if (pfn) pfn(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT* pTagInfo) {
    static PFN_vkDebugMarkerSetObjectTagEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDebugMarkerSetObjectTagEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDebugMarkerSetObjectTagEXT");
    return pfn ? pfn(device, pTagInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo) {
    static PFN_vkDebugMarkerSetObjectNameEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDebugMarkerSetObjectNameEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDebugMarkerSetObjectNameEXT");
    return pfn ? pfn(device, pNameInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    static PFN_vkCmdDebugMarkerBeginEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDebugMarkerBeginEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDebugMarkerBeginEXT");
    if (pfn) pfn(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdDebugMarkerEndEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDebugMarkerEndEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDebugMarkerEndEXT");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) {
    static PFN_vkCmdDebugMarkerInsertEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDebugMarkerInsertEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDebugMarkerInsertEXT");
    if (pfn) pfn(commandBuffer, pMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) {
    static PFN_vkCmdBindTransformFeedbackBuffersEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindTransformFeedbackBuffersEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindTransformFeedbackBuffersEXT");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    static PFN_vkCmdBeginTransformFeedbackEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginTransformFeedbackEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginTransformFeedbackEXT");
    if (pfn) pfn(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) {
    static PFN_vkCmdEndTransformFeedbackEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndTransformFeedbackEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndTransformFeedbackEXT");
    if (pfn) pfn(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    static PFN_vkCmdBeginQueryIndexedEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginQueryIndexedEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginQueryIndexedEXT");
    if (pfn) pfn(commandBuffer, queryPool, query, flags, index);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) {
    static PFN_vkCmdEndQueryIndexedEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndQueryIndexedEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndQueryIndexedEXT");
    if (pfn) pfn(commandBuffer, queryPool, query, index);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    static PFN_vkCmdDrawIndirectByteCountEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectByteCountEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectByteCountEXT");
    if (pfn) pfn(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCuModuleNVX(VkDevice device, const VkCuModuleCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) {
    static PFN_vkCreateCuModuleNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateCuModuleNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateCuModuleNVX");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pModule) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCuFunctionNVX(VkDevice device, const VkCuFunctionCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) {
    static PFN_vkCreateCuFunctionNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateCuFunctionNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateCuFunctionNVX");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pFunction) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCuModuleNVX(VkDevice device, VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyCuModuleNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyCuModuleNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyCuModuleNVX");
    if (pfn) pfn(device, module, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCuFunctionNVX(VkDevice device, VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyCuFunctionNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyCuFunctionNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyCuFunctionNVX");
    if (pfn) pfn(device, function, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) {
    static PFN_vkCmdCuLaunchKernelNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCuLaunchKernelNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCuLaunchKernelNVX");
    if (pfn) pfn(commandBuffer, pLaunchInfo);
}

VKAPI_ATTR uint32_t VKAPI_CALL vkGetImageViewHandleNVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    static PFN_vkGetImageViewHandleNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageViewHandleNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageViewHandleNVX");
    return pfn ? pfn(device, pInfo) : (uint32_t)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetImageViewHandle64NVX(VkDevice device, const VkImageViewHandleInfoNVX* pInfo) {
    static PFN_vkGetImageViewHandle64NVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageViewHandle64NVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageViewHandle64NVX");
    return pfn ? pfn(device, pInfo) : (uint64_t)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetImageViewAddressNVX(VkDevice device, VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) {
    static PFN_vkGetImageViewAddressNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageViewAddressNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageViewAddressNVX");
    return pfn ? pfn(device, imageView, pProperties) : (VkResult)0;
}

VKAPI_ATTR uint64_t VKAPI_CALL vkGetDeviceCombinedImageSamplerIndexNVX(VkDevice device, uint64_t imageViewIndex, uint64_t samplerIndex) {
    static PFN_vkGetDeviceCombinedImageSamplerIndexNVX pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceCombinedImageSamplerIndexNVX)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceCombinedImageSamplerIndexNVX");
    return pfn ? pfn(device, imageViewIndex, samplerIndex) : (uint64_t)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndirectCountAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndirectCountAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndirectCountAMD");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawIndexedIndirectCountAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawIndexedIndirectCountAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawIndexedIndirectCountAMD");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetShaderInfoAMD(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) {
    static PFN_vkGetShaderInfoAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetShaderInfoAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetShaderInfoAMD");
    return pfn ? pfn(device, pipeline, shaderStage, infoType, pInfoSize, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
    return pfn ? pfn(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) {
    static PFN_vkCmdBeginConditionalRenderingEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginConditionalRenderingEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginConditionalRenderingEXT");
    if (pfn) pfn(commandBuffer, pConditionalRenderingBegin);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndConditionalRenderingEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndConditionalRenderingEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndConditionalRenderingEXT");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) {
    static PFN_vkCmdSetViewportWScalingNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportWScalingNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportWScalingNV");
    if (pfn) pfn(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}

VKAPI_ATTR VkResult VKAPI_CALL vkReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) {
    static PFN_vkReleaseDisplayEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleaseDisplayEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleaseDisplayEXT");
    return pfn ? pfn(physicalDevice, display) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) {
    static PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    return pfn ? pfn(physicalDevice, surface, pSurfaceCapabilities) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkDisplayPowerControlEXT(VkDevice device, VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) {
    static PFN_vkDisplayPowerControlEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDisplayPowerControlEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDisplayPowerControlEXT");
    return pfn ? pfn(device, display, pDisplayPowerInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDeviceEventEXT(VkDevice device, const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    static PFN_vkRegisterDeviceEventEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkRegisterDeviceEventEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkRegisterDeviceEventEXT");
    return pfn ? pfn(device, pDeviceEventInfo, pAllocator, pFence) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDisplayEventEXT(VkDevice device, VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    static PFN_vkRegisterDisplayEventEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkRegisterDisplayEventEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkRegisterDisplayEventEXT");
    return pfn ? pfn(device, display, pDisplayEventInfo, pAllocator, pFence) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainCounterEXT(VkDevice device, VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) {
    static PFN_vkGetSwapchainCounterEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSwapchainCounterEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSwapchainCounterEXT");
    return pfn ? pfn(device, swapchain, counter, pCounterValue) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    static PFN_vkGetRefreshCycleDurationGOOGLE pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRefreshCycleDurationGOOGLE)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRefreshCycleDurationGOOGLE");
    return pfn ? pfn(device, swapchain, pDisplayTimingProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    static PFN_vkGetPastPresentationTimingGOOGLE pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPastPresentationTimingGOOGLE)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPastPresentationTimingGOOGLE");
    return pfn ? pfn(device, swapchain, pPresentationTimingCount, pPresentationTimings) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) {
    static PFN_vkCmdSetDiscardRectangleEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDiscardRectangleEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDiscardRectangleEXT");
    if (pfn) pfn(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable) {
    static PFN_vkCmdSetDiscardRectangleEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDiscardRectangleEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDiscardRectangleEnableEXT");
    if (pfn) pfn(commandBuffer, discardRectangleEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode) {
    static PFN_vkCmdSetDiscardRectangleModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDiscardRectangleModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDiscardRectangleModeEXT");
    if (pfn) pfn(commandBuffer, discardRectangleMode);
}

VKAPI_ATTR void VKAPI_CALL vkSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    static PFN_vkSetHdrMetadataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetHdrMetadataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetHdrMetadataEXT");
    if (pfn) pfn(device, swapchainCount, pSwapchains, pMetadata);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    static PFN_vkSetDebugUtilsObjectNameEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetDebugUtilsObjectNameEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetDebugUtilsObjectNameEXT");
    return pfn ? pfn(device, pNameInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) {
    static PFN_vkSetDebugUtilsObjectTagEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetDebugUtilsObjectTagEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetDebugUtilsObjectTagEXT");
    return pfn ? pfn(device, pTagInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    static PFN_vkQueueBeginDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueBeginDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueBeginDebugUtilsLabelEXT");
    if (pfn) pfn(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    static PFN_vkQueueEndDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueEndDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueEndDebugUtilsLabelEXT");
    if (pfn) pfn(queue);
}

VKAPI_ATTR void VKAPI_CALL vkQueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    static PFN_vkQueueInsertDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueInsertDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueInsertDebugUtilsLabelEXT");
    if (pfn) pfn(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    static PFN_vkCmdBeginDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginDebugUtilsLabelEXT");
    if (pfn) pfn(commandBuffer, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndDebugUtilsLabelEXT");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
    static PFN_vkCmdInsertDebugUtilsLabelEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdInsertDebugUtilsLabelEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdInsertDebugUtilsLabelEXT");
    if (pfn) pfn(commandBuffer, pLabelInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    static PFN_vkCreateDebugUtilsMessengerEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDebugUtilsMessengerEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDebugUtilsMessengerEXT");
    return pfn ? pfn(instance, pCreateInfo, pAllocator, pMessenger) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDebugUtilsMessengerEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDebugUtilsMessengerEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
    if (pfn) pfn(instance, messenger, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    static PFN_vkSubmitDebugUtilsMessageEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSubmitDebugUtilsMessageEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSubmitDebugUtilsMessageEXT");
    if (pfn) pfn(instance, messageSeverity, messageTypes, pCallbackData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateGpaSessionAMD(VkDevice device, const VkGpaSessionCreateInfoAMD* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkGpaSessionAMD* pGpaSession) {
    static PFN_vkCreateGpaSessionAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateGpaSessionAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateGpaSessionAMD");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pGpaSession) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyGpaSessionAMD(VkDevice device, VkGpaSessionAMD gpaSession, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyGpaSessionAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyGpaSessionAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyGpaSessionAMD");
    if (pfn) pfn(device, gpaSession, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetGpaDeviceClockModeAMD(VkDevice device, VkGpaDeviceClockModeInfoAMD* pInfo) {
    static PFN_vkSetGpaDeviceClockModeAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetGpaDeviceClockModeAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetGpaDeviceClockModeAMD");
    return pfn ? pfn(device, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetGpaDeviceClockInfoAMD(VkDevice device, VkGpaDeviceGetClockInfoAMD* pInfo) {
    static PFN_vkGetGpaDeviceClockInfoAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetGpaDeviceClockInfoAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetGpaDeviceClockInfoAMD");
    return pfn ? pfn(device, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdBeginGpaSessionAMD(VkCommandBuffer commandBuffer, VkGpaSessionAMD gpaSession) {
    static PFN_vkCmdBeginGpaSessionAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginGpaSessionAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginGpaSessionAMD");
    return pfn ? pfn(commandBuffer, gpaSession) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdEndGpaSessionAMD(VkCommandBuffer commandBuffer, VkGpaSessionAMD gpaSession) {
    static PFN_vkCmdEndGpaSessionAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndGpaSessionAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndGpaSessionAMD");
    return pfn ? pfn(commandBuffer, gpaSession) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdBeginGpaSampleAMD(VkCommandBuffer commandBuffer, VkGpaSessionAMD gpaSession, const VkGpaSampleBeginInfoAMD* pGpaSampleBeginInfo, uint32_t* pSampleID) {
    static PFN_vkCmdBeginGpaSampleAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginGpaSampleAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginGpaSampleAMD");
    return pfn ? pfn(commandBuffer, gpaSession, pGpaSampleBeginInfo, pSampleID) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndGpaSampleAMD(VkCommandBuffer commandBuffer, VkGpaSessionAMD gpaSession, uint32_t sampleID) {
    static PFN_vkCmdEndGpaSampleAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndGpaSampleAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndGpaSampleAMD");
    if (pfn) pfn(commandBuffer, gpaSession, sampleID);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetGpaSessionStatusAMD(VkDevice device, VkGpaSessionAMD gpaSession) {
    static PFN_vkGetGpaSessionStatusAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetGpaSessionStatusAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetGpaSessionStatusAMD");
    return pfn ? pfn(device, gpaSession) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetGpaSessionResultsAMD(VkDevice device, VkGpaSessionAMD gpaSession, uint32_t sampleID, size_t* pSizeInBytes, void* pData) {
    static PFN_vkGetGpaSessionResultsAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetGpaSessionResultsAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetGpaSessionResultsAMD");
    return pfn ? pfn(device, gpaSession, sampleID, pSizeInBytes, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetGpaSessionAMD(VkDevice device, VkGpaSessionAMD gpaSession) {
    static PFN_vkResetGpaSessionAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetGpaSessionAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetGpaSessionAMD");
    return pfn ? pfn(device, gpaSession) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyGpaSessionResultsAMD(VkCommandBuffer commandBuffer, VkGpaSessionAMD gpaSession) {
    static PFN_vkCmdCopyGpaSessionResultsAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyGpaSessionResultsAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyGpaSessionResultsAMD");
    if (pfn) pfn(commandBuffer, gpaSession);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWriteSamplerDescriptorsEXT(VkDevice device, uint32_t samplerCount, const VkSamplerCreateInfo* pSamplers, const VkHostAddressRangeEXT* pDescriptors) {
    static PFN_vkWriteSamplerDescriptorsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkWriteSamplerDescriptorsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWriteSamplerDescriptorsEXT");
    return pfn ? pfn(device, samplerCount, pSamplers, pDescriptors) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWriteResourceDescriptorsEXT(VkDevice device, uint32_t resourceCount, const VkResourceDescriptorInfoEXT* pResources, const VkHostAddressRangeEXT* pDescriptors) {
    static PFN_vkWriteResourceDescriptorsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkWriteResourceDescriptorsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWriteResourceDescriptorsEXT");
    return pfn ? pfn(device, resourceCount, pResources, pDescriptors) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindSamplerHeapEXT(VkCommandBuffer commandBuffer, const VkBindHeapInfoEXT* pBindInfo) {
    static PFN_vkCmdBindSamplerHeapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindSamplerHeapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindSamplerHeapEXT");
    if (pfn) pfn(commandBuffer, pBindInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindResourceHeapEXT(VkCommandBuffer commandBuffer, const VkBindHeapInfoEXT* pBindInfo) {
    static PFN_vkCmdBindResourceHeapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindResourceHeapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindResourceHeapEXT");
    if (pfn) pfn(commandBuffer, pBindInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushDataEXT(VkCommandBuffer commandBuffer, const VkPushDataInfoEXT* pPushDataInfo) {
    static PFN_vkCmdPushDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPushDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPushDataEXT");
    if (pfn) pfn(commandBuffer, pPushDataInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetImageOpaqueCaptureDataEXT(VkDevice device, uint32_t imageCount, const VkImage* pImages, VkHostAddressRangeEXT* pDatas) {
    static PFN_vkGetImageOpaqueCaptureDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageOpaqueCaptureDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageOpaqueCaptureDataEXT");
    return pfn ? pfn(device, imageCount, pImages, pDatas) : (VkResult)0;
}

VKAPI_ATTR VkDeviceSize VKAPI_CALL vkGetPhysicalDeviceDescriptorSizeEXT(VkPhysicalDevice physicalDevice, VkDescriptorType descriptorType) {
    static PFN_vkGetPhysicalDeviceDescriptorSizeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceDescriptorSizeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceDescriptorSizeEXT");
    return pfn ? pfn(physicalDevice, descriptorType) : (VkDeviceSize)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkRegisterCustomBorderColorEXT(VkDevice device, const VkSamplerCustomBorderColorCreateInfoEXT* pBorderColor, VkBool32 requestIndex, uint32_t* pIndex) {
    static PFN_vkRegisterCustomBorderColorEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkRegisterCustomBorderColorEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkRegisterCustomBorderColorEXT");
    return pfn ? pfn(device, pBorderColor, requestIndex, pIndex) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkUnregisterCustomBorderColorEXT(VkDevice device, uint32_t index) {
    static PFN_vkUnregisterCustomBorderColorEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkUnregisterCustomBorderColorEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUnregisterCustomBorderColorEXT");
    if (pfn) pfn(device, index);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetTensorOpaqueCaptureDataARM(VkDevice device, uint32_t tensorCount, const VkTensorARM* pTensors, VkHostAddressRangeEXT* pDatas) {
    static PFN_vkGetTensorOpaqueCaptureDataARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetTensorOpaqueCaptureDataARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetTensorOpaqueCaptureDataARM");
    return pfn ? pfn(device, tensorCount, pTensors, pDatas) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) {
    static PFN_vkCmdSetSampleLocationsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetSampleLocationsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetSampleLocationsEXT");
    if (pfn) pfn(commandBuffer, pSampleLocationsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) {
    static PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    if (pfn) pfn(physicalDevice, samples, pMultisampleProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetImageDrmFormatModifierPropertiesEXT(VkDevice device, VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) {
    static PFN_vkGetImageDrmFormatModifierPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageDrmFormatModifierPropertiesEXT");
    return pfn ? pfn(device, image, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) {
    static PFN_vkCreateValidationCacheEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateValidationCacheEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateValidationCacheEXT");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pValidationCache) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyValidationCacheEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyValidationCacheEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyValidationCacheEXT");
    if (pfn) pfn(device, validationCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches) {
    static PFN_vkMergeValidationCachesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkMergeValidationCachesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkMergeValidationCachesEXT");
    return pfn ? pfn(device, dstCache, srcCacheCount, pSrcCaches) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData) {
    static PFN_vkGetValidationCacheDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetValidationCacheDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetValidationCacheDataEXT");
    return pfn ? pfn(device, validationCache, pDataSize, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    static PFN_vkCmdBindShadingRateImageNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindShadingRateImageNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindShadingRateImageNV");
    if (pfn) pfn(commandBuffer, imageView, imageLayout);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) {
    static PFN_vkCmdSetViewportShadingRatePaletteNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportShadingRatePaletteNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportShadingRatePaletteNV");
    if (pfn) pfn(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) {
    static PFN_vkCmdSetCoarseSampleOrderNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoarseSampleOrderNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoarseSampleOrderNV");
    if (pfn) pfn(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) {
    static PFN_vkCreateAccelerationStructureNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateAccelerationStructureNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateAccelerationStructureNV");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pAccelerationStructure) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyAccelerationStructureNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyAccelerationStructureNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyAccelerationStructureNV");
    if (pfn) pfn(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureMemoryRequirementsNV(VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetAccelerationStructureMemoryRequirementsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetAccelerationStructureMemoryRequirementsNV");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    static PFN_vkBindAccelerationStructureMemoryNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindAccelerationStructureMemoryNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindAccelerationStructureMemoryNV");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    static PFN_vkCmdBuildAccelerationStructureNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildAccelerationStructureNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildAccelerationStructureNV");
    if (pfn) pfn(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) {
    static PFN_vkCmdCopyAccelerationStructureNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyAccelerationStructureNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyAccelerationStructureNV");
    if (pfn) pfn(commandBuffer, dst, src, mode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) {
    static PFN_vkCmdTraceRaysNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdTraceRaysNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdTraceRaysNV");
    if (pfn) pfn(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    static PFN_vkCreateRayTracingPipelinesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateRayTracingPipelinesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateRayTracingPipelinesNV");
    return pfn ? pfn(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    static PFN_vkGetRayTracingShaderGroupHandlesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRayTracingShaderGroupHandlesKHR");
    return pfn ? pfn(device, pipeline, firstGroup, groupCount, dataSize, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingShaderGroupHandlesNV(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    static PFN_vkGetRayTracingShaderGroupHandlesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRayTracingShaderGroupHandlesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRayTracingShaderGroupHandlesNV");
    return pfn ? pfn(device, pipeline, firstGroup, groupCount, dataSize, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure, size_t dataSize, void* pData) {
    static PFN_vkGetAccelerationStructureHandleNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetAccelerationStructureHandleNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetAccelerationStructureHandleNV");
    return pfn ? pfn(device, accelerationStructure, dataSize, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    static PFN_vkCmdWriteAccelerationStructuresPropertiesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteAccelerationStructuresPropertiesNV");
    if (pfn) pfn(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCompileDeferredNV(VkDevice device, VkPipeline pipeline, uint32_t shader) {
    static PFN_vkCompileDeferredNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCompileDeferredNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCompileDeferredNV");
    return pfn ? pfn(device, pipeline, shader) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) {
    static PFN_vkGetMemoryHostPointerPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetMemoryHostPointerPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetMemoryHostPointerPropertiesEXT");
    return pfn ? pfn(device, handleType, pHostPointer, pMemoryHostPointerProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    static PFN_vkCmdWriteBufferMarkerAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteBufferMarkerAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteBufferMarkerAMD");
    if (pfn) pfn(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    static PFN_vkCmdWriteBufferMarker2AMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteBufferMarker2AMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteBufferMarker2AMD");
    if (pfn) pfn(commandBuffer, stage, dstBuffer, dstOffset, marker);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainKHR* pTimeDomains) {
    static PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    return pfn ? pfn(physicalDevice, pTimeDomainCount, pTimeDomains) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount, const VkCalibratedTimestampInfoKHR* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) {
    static PFN_vkGetCalibratedTimestampsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetCalibratedTimestampsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetCalibratedTimestampsEXT");
    return pfn ? pfn(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) {
    static PFN_vkCmdDrawMeshTasksNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksNV");
    if (pfn) pfn(commandBuffer, taskCount, firstTask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    static PFN_vkCmdDrawMeshTasksIndirectNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirectNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirectNV");
    if (pfn) pfn(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawMeshTasksIndirectCountNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirectCountNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirectCountNV");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables) {
    static PFN_vkCmdSetExclusiveScissorEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetExclusiveScissorEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetExclusiveScissorEnableNV");
    if (pfn) pfn(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissorEnables);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) {
    static PFN_vkCmdSetExclusiveScissorNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetExclusiveScissorNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetExclusiveScissorNV");
    if (pfn) pfn(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) {
    static PFN_vkCmdSetCheckpointNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCheckpointNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCheckpointNV");
    if (pfn) pfn(commandBuffer, pCheckpointMarker);
}

VKAPI_ATTR void VKAPI_CALL vkGetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) {
    static PFN_vkGetQueueCheckpointDataNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetQueueCheckpointDataNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetQueueCheckpointDataNV");
    if (pfn) pfn(queue, pCheckpointDataCount, pCheckpointData);
}

VKAPI_ATTR void VKAPI_CALL vkGetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) {
    static PFN_vkGetQueueCheckpointData2NV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetQueueCheckpointData2NV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetQueueCheckpointData2NV");
    if (pfn) pfn(queue, pCheckpointDataCount, pCheckpointData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetSwapchainPresentTimingQueueSizeEXT(VkDevice device, VkSwapchainKHR swapchain, uint32_t size) {
    static PFN_vkSetSwapchainPresentTimingQueueSizeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetSwapchainPresentTimingQueueSizeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetSwapchainPresentTimingQueueSizeEXT");
    return pfn ? pfn(device, swapchain, size) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainTimingPropertiesEXT(VkDevice device, VkSwapchainKHR swapchain, VkSwapchainTimingPropertiesEXT* pSwapchainTimingProperties, uint64_t* pSwapchainTimingPropertiesCounter) {
    static PFN_vkGetSwapchainTimingPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSwapchainTimingPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSwapchainTimingPropertiesEXT");
    return pfn ? pfn(device, swapchain, pSwapchainTimingProperties, pSwapchainTimingPropertiesCounter) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainTimeDomainPropertiesEXT(VkDevice device, VkSwapchainKHR swapchain, VkSwapchainTimeDomainPropertiesEXT* pSwapchainTimeDomainProperties, uint64_t* pTimeDomainsCounter) {
    static PFN_vkGetSwapchainTimeDomainPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSwapchainTimeDomainPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSwapchainTimeDomainPropertiesEXT");
    return pfn ? pfn(device, swapchain, pSwapchainTimeDomainProperties, pTimeDomainsCounter) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPastPresentationTimingEXT(VkDevice device, const VkPastPresentationTimingInfoEXT* pPastPresentationTimingInfo, VkPastPresentationTimingPropertiesEXT* pPastPresentationTimingProperties) {
    static PFN_vkGetPastPresentationTimingEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPastPresentationTimingEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPastPresentationTimingEXT");
    return pfn ? pfn(device, pPastPresentationTimingInfo, pPastPresentationTimingProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkInitializePerformanceApiINTEL(VkDevice device, const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) {
    static PFN_vkInitializePerformanceApiINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkInitializePerformanceApiINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkInitializePerformanceApiINTEL");
    return pfn ? pfn(device, pInitializeInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkUninitializePerformanceApiINTEL(VkDevice device) {
    static PFN_vkUninitializePerformanceApiINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkUninitializePerformanceApiINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUninitializePerformanceApiINTEL");
    if (pfn) pfn(device);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) {
    static PFN_vkCmdSetPerformanceMarkerINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPerformanceMarkerINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPerformanceMarkerINTEL");
    return pfn ? pfn(commandBuffer, pMarkerInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) {
    static PFN_vkCmdSetPerformanceStreamMarkerINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPerformanceStreamMarkerINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPerformanceStreamMarkerINTEL");
    return pfn ? pfn(commandBuffer, pMarkerInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) {
    static PFN_vkCmdSetPerformanceOverrideINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPerformanceOverrideINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPerformanceOverrideINTEL");
    return pfn ? pfn(commandBuffer, pOverrideInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquirePerformanceConfigurationINTEL(VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) {
    static PFN_vkAcquirePerformanceConfigurationINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkAcquirePerformanceConfigurationINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAcquirePerformanceConfigurationINTEL");
    return pfn ? pfn(device, pAcquireInfo, pConfiguration) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkReleasePerformanceConfigurationINTEL(VkDevice device, VkPerformanceConfigurationINTEL configuration) {
    static PFN_vkReleasePerformanceConfigurationINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleasePerformanceConfigurationINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleasePerformanceConfigurationINTEL");
    return pfn ? pfn(device, configuration) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) {
    static PFN_vkQueueSetPerformanceConfigurationINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueSetPerformanceConfigurationINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueSetPerformanceConfigurationINTEL");
    return pfn ? pfn(queue, configuration) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPerformanceParameterINTEL(VkDevice device, VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) {
    static PFN_vkGetPerformanceParameterINTEL pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPerformanceParameterINTEL)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPerformanceParameterINTEL");
    return pfn ? pfn(device, parameter, pValue) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkSetLocalDimmingAMD(VkDevice device, VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) {
    static PFN_vkSetLocalDimmingAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetLocalDimmingAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetLocalDimmingAMD");
    if (pfn) pfn(device, swapChain, localDimmingEnable);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo) {
    static PFN_vkGetBufferDeviceAddressEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferDeviceAddressEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferDeviceAddressEXT");
    return pfn ? pfn(device, pInfo) : (VkDeviceAddress)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolProperties* pToolProperties) {
    static PFN_vkGetPhysicalDeviceToolPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceToolPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceToolPropertiesEXT");
    return pfn ? pfn(physicalDevice, pToolCount, pToolProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) {
    static PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) {
    static PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
    return pfn ? pfn(physicalDevice, pCombinationCount, pCombinations) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    static PFN_vkCreateHeadlessSurfaceEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateHeadlessSurfaceEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateHeadlessSurfaceEXT");
    return pfn ? pfn(instance, pCreateInfo, pAllocator, pSurface) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) {
    static PFN_vkCmdSetLineStippleEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineStippleEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineStippleEXT");
    if (pfn) pfn(commandBuffer, lineStippleFactor, lineStipplePattern);
}

VKAPI_ATTR void VKAPI_CALL vkResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    static PFN_vkResetQueryPoolEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkResetQueryPoolEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkResetQueryPoolEXT");
    if (pfn) pfn(device, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    static PFN_vkCmdSetCullModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCullModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCullModeEXT");
    if (pfn) pfn(commandBuffer, cullMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    static PFN_vkCmdSetFrontFaceEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetFrontFaceEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetFrontFaceEXT");
    if (pfn) pfn(commandBuffer, frontFace);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) {
    static PFN_vkCmdSetPrimitiveTopologyEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPrimitiveTopologyEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPrimitiveTopologyEXT");
    if (pfn) pfn(commandBuffer, primitiveTopology);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) {
    static PFN_vkCmdSetViewportWithCountEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportWithCountEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportWithCountEXT");
    if (pfn) pfn(commandBuffer, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) {
    static PFN_vkCmdSetScissorWithCountEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetScissorWithCountEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetScissorWithCountEXT");
    if (pfn) pfn(commandBuffer, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) {
    static PFN_vkCmdBindVertexBuffers2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindVertexBuffers2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindVertexBuffers2EXT");
    if (pfn) pfn(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    static PFN_vkCmdSetDepthTestEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthTestEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthTestEnableEXT");
    if (pfn) pfn(commandBuffer, depthTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    static PFN_vkCmdSetDepthWriteEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthWriteEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthWriteEnableEXT");
    if (pfn) pfn(commandBuffer, depthWriteEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    static PFN_vkCmdSetDepthCompareOpEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthCompareOpEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthCompareOpEXT");
    if (pfn) pfn(commandBuffer, depthCompareOp);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) {
    static PFN_vkCmdSetDepthBoundsTestEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBoundsTestEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBoundsTestEnableEXT");
    if (pfn) pfn(commandBuffer, depthBoundsTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    static PFN_vkCmdSetStencilTestEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilTestEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilTestEnableEXT");
    if (pfn) pfn(commandBuffer, stencilTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) {
    static PFN_vkCmdSetStencilOpEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetStencilOpEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetStencilOpEXT");
    if (pfn) pfn(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfo* pCopyMemoryToImageInfo) {
    static PFN_vkCopyMemoryToImageEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMemoryToImageEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMemoryToImageEXT");
    return pfn ? pfn(device, pCopyMemoryToImageInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfo* pCopyImageToMemoryInfo) {
    static PFN_vkCopyImageToMemoryEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyImageToMemoryEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyImageToMemoryEXT");
    return pfn ? pfn(device, pCopyImageToMemoryInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfo* pCopyImageToImageInfo) {
    static PFN_vkCopyImageToImageEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyImageToImageEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyImageToImageEXT");
    return pfn ? pfn(device, pCopyImageToImageInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkTransitionImageLayoutEXT(VkDevice device, uint32_t transitionCount, const VkHostImageLayoutTransitionInfo* pTransitions) {
    static PFN_vkTransitionImageLayoutEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkTransitionImageLayoutEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkTransitionImageLayoutEXT");
    return pfn ? pfn(device, transitionCount, pTransitions) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout2EXT(VkDevice device, VkImage image, const VkImageSubresource2* pSubresource, VkSubresourceLayout2* pLayout) {
    static PFN_vkGetImageSubresourceLayout2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageSubresourceLayout2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageSubresourceLayout2EXT");
    if (pfn) pfn(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoKHR* pReleaseInfo) {
    static PFN_vkReleaseSwapchainImagesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkReleaseSwapchainImagesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkReleaseSwapchainImagesEXT");
    return pfn ? pfn(device, pReleaseInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetGeneratedCommandsMemoryRequirementsNV(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetGeneratedCommandsMemoryRequirementsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetGeneratedCommandsMemoryRequirementsNV");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    static PFN_vkCmdPreprocessGeneratedCommandsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPreprocessGeneratedCommandsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPreprocessGeneratedCommandsNV");
    if (pfn) pfn(commandBuffer, pGeneratedCommandsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) {
    static PFN_vkCmdExecuteGeneratedCommandsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdExecuteGeneratedCommandsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdExecuteGeneratedCommandsNV");
    if (pfn) pfn(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) {
    static PFN_vkCmdBindPipelineShaderGroupNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindPipelineShaderGroupNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindPipelineShaderGroupNV");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateIndirectCommandsLayoutNV(VkDevice device, const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) {
    static PFN_vkCreateIndirectCommandsLayoutNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateIndirectCommandsLayoutNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateIndirectCommandsLayoutNV");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyIndirectCommandsLayoutNV(VkDevice device, VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyIndirectCommandsLayoutNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyIndirectCommandsLayoutNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyIndirectCommandsLayoutNV");
    if (pfn) pfn(device, indirectCommandsLayout, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo) {
    static PFN_vkCmdSetDepthBias2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBias2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBias2EXT");
    if (pfn) pfn(commandBuffer, pDepthBiasInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) {
    static PFN_vkAcquireDrmDisplayEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkAcquireDrmDisplayEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAcquireDrmDisplayEXT");
    return pfn ? pfn(physicalDevice, drmFd, display) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display) {
    static PFN_vkGetDrmDisplayEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDrmDisplayEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDrmDisplayEXT");
    return pfn ? pfn(physicalDevice, drmFd, connectorId, display) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) {
    static PFN_vkCreatePrivateDataSlotEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreatePrivateDataSlotEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreatePrivateDataSlotEXT");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pPrivateDataSlot) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPrivateDataSlotEXT(VkDevice device, VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyPrivateDataSlotEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyPrivateDataSlotEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyPrivateDataSlotEXT");
    if (pfn) pfn(device, privateDataSlot, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data) {
    static PFN_vkSetPrivateDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetPrivateDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetPrivateDataEXT");
    return pfn ? pfn(device, objectType, objectHandle, privateDataSlot, data) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPrivateDataEXT(VkDevice device, VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t* pData) {
    static PFN_vkGetPrivateDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPrivateDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPrivateDataEXT");
    if (pfn) pfn(device, objectType, objectHandle, privateDataSlot, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSetPerfHintQCOM(VkQueue queue, const VkPerfHintInfoQCOM* pPerfHintInfo) {
    static PFN_vkQueueSetPerfHintQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueSetPerfHintQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueSetPerfHintQCOM");
    return pfn ? pfn(queue, pPerfHintInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchTileQCOM(VkCommandBuffer commandBuffer, const VkDispatchTileInfoQCOM* pDispatchTileInfo) {
    static PFN_vkCmdDispatchTileQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchTileQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchTileQCOM");
    if (pfn) pfn(commandBuffer, pDispatchTileInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileBeginInfoQCOM* pPerTileBeginInfo) {
    static PFN_vkCmdBeginPerTileExecutionQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginPerTileExecutionQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginPerTileExecutionQCOM");
    if (pfn) pfn(commandBuffer, pPerTileBeginInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndPerTileExecutionQCOM(VkCommandBuffer commandBuffer, const VkPerTileEndInfoQCOM* pPerTileEndInfo) {
    static PFN_vkCmdEndPerTileExecutionQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndPerTileExecutionQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndPerTileExecutionQCOM");
    if (pfn) pfn(commandBuffer, pPerTileEndInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout, VkDeviceSize* pLayoutSizeInBytes) {
    static PFN_vkGetDescriptorSetLayoutSizeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetLayoutSizeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetLayoutSizeEXT");
    if (pfn) pfn(device, layout, pLayoutSizeInBytes);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutBindingOffsetEXT(VkDevice device, VkDescriptorSetLayout layout, uint32_t binding, VkDeviceSize* pOffset) {
    static PFN_vkGetDescriptorSetLayoutBindingOffsetEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    if (pfn) pfn(device, layout, binding, pOffset);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT* pDescriptorInfo, size_t dataSize, void* pDescriptor) {
    static PFN_vkGetDescriptorEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorEXT");
    if (pfn) pfn(device, pDescriptorInfo, dataSize, pDescriptor);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount, const VkDescriptorBufferBindingInfoEXT* pBindingInfos) {
    static PFN_vkCmdBindDescriptorBuffersEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorBuffersEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorBuffersEXT");
    if (pfn) pfn(commandBuffer, bufferCount, pBindingInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets) {
    static PFN_vkCmdSetDescriptorBufferOffsetsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDescriptorBufferOffsetsEXT");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorBufferEmbeddedSamplersEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set) {
    static PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, layout, set);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetBufferOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkBufferCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    static PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetImageOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    static PFN_vkGetImageOpaqueCaptureDescriptorDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetImageViewOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkImageViewCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    static PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSamplerOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkSamplerCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    static PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(VkDevice device, const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo, void* pData) {
    static PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate, const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) {
    static PFN_vkCmdSetFragmentShadingRateEnumNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetFragmentShadingRateEnumNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetFragmentShadingRateEnumNV");
    if (pfn) pfn(commandBuffer, shadingRate, combinerOps);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceFaultInfoEXT(VkDevice device, VkDeviceFaultCountsEXT* pFaultCounts, VkDeviceFaultInfoEXT* pFaultInfo) {
    static PFN_vkGetDeviceFaultInfoEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceFaultInfoEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceFaultInfoEXT");
    return pfn ? pfn(device, pFaultCounts, pFaultInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    static PFN_vkCmdSetVertexInputEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetVertexInputEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetVertexInputEXT");
    if (pfn) pfn(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkDevice device, VkRenderPass renderpass, VkExtent2D* pMaxWorkgroupSize) {
    static PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
    return pfn ? pfn(device, renderpass, pMaxWorkgroupSize) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdSubpassShadingHUAWEI pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSubpassShadingHUAWEI)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSubpassShadingHUAWEI");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) {
    static PFN_vkCmdBindInvocationMaskHUAWEI pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindInvocationMaskHUAWEI)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindInvocationMaskHUAWEI");
    if (pfn) pfn(commandBuffer, imageView, imageLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryRemoteAddressNV(VkDevice device, const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo, VkRemoteAddressNV* pAddress) {
    static PFN_vkGetMemoryRemoteAddressNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetMemoryRemoteAddressNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetMemoryRemoteAddressNV");
    return pfn ? pfn(device, pMemoryGetRemoteAddressInfo, pAddress) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoKHR* pPipelineInfo, VkBaseOutStructure* pPipelineProperties) {
    static PFN_vkGetPipelinePropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelinePropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelinePropertiesEXT");
    return pfn ? pfn(device, pPipelineInfo, pPipelineProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    static PFN_vkCmdSetPatchControlPointsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPatchControlPointsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPatchControlPointsEXT");
    if (pfn) pfn(commandBuffer, patchControlPoints);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) {
    static PFN_vkCmdSetRasterizerDiscardEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRasterizerDiscardEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRasterizerDiscardEnableEXT");
    if (pfn) pfn(commandBuffer, rasterizerDiscardEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    static PFN_vkCmdSetDepthBiasEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthBiasEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthBiasEnableEXT");
    if (pfn) pfn(commandBuffer, depthBiasEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    static PFN_vkCmdSetLogicOpEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLogicOpEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLogicOpEXT");
    if (pfn) pfn(commandBuffer, logicOp);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) {
    static PFN_vkCmdSetPrimitiveRestartEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPrimitiveRestartEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPrimitiveRestartEnableEXT");
    if (pfn) pfn(commandBuffer, primitiveRestartEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkBool32* pColorWriteEnables) {
    static PFN_vkCmdSetColorWriteEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetColorWriteEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetColorWriteEnableEXT");
    if (pfn) pfn(commandBuffer, attachmentCount, pColorWriteEnables);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    static PFN_vkCmdDrawMultiEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMultiEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMultiEXT");
    if (pfn) pfn(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    static PFN_vkCmdDrawMultiIndexedEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMultiIndexedEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMultiIndexedEXT");
    if (pfn) pfn(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkMicromapEXT* pMicromap) {
    static PFN_vkCreateMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateMicromapEXT");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pMicromap) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyMicromapEXT");
    if (pfn) pfn(device, micromap, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
    static PFN_vkCmdBuildMicromapsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildMicromapsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildMicromapsEXT");
    if (pfn) pfn(commandBuffer, infoCount, pInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkMicromapBuildInfoEXT* pInfos) {
    static PFN_vkBuildMicromapsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkBuildMicromapsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBuildMicromapsEXT");
    return pfn ? pfn(device, deferredOperation, infoCount, pInfos) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapInfoEXT* pInfo) {
    static PFN_vkCopyMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMicromapEXT");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    static PFN_vkCopyMicromapToMemoryEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMicromapToMemoryEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMicromapToMemoryEXT");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    static PFN_vkCopyMemoryToMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMemoryToMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMemoryToMicromapEXT");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT* pMicromaps, VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    static PFN_vkWriteMicromapsPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkWriteMicromapsPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWriteMicromapsPropertiesEXT");
    return pfn ? pfn(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT* pInfo) {
    static PFN_vkCmdCopyMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMicromapEXT");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT* pInfo) {
    static PFN_vkCmdCopyMicromapToMemoryEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMicromapToMemoryEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMicromapToMemoryEXT");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT* pInfo) {
    static PFN_vkCmdCopyMemoryToMicromapEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryToMicromapEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryToMicromapEXT");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount, const VkMicromapEXT* pMicromaps, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    static PFN_vkCmdWriteMicromapsPropertiesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteMicromapsPropertiesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteMicromapsPropertiesEXT");
    if (pfn) pfn(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    static PFN_vkGetDeviceMicromapCompatibilityEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceMicromapCompatibilityEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceMicromapCompatibilityEXT");
    if (pfn) pfn(device, pVersionInfo, pCompatibility);
}

VKAPI_ATTR void VKAPI_CALL vkGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkMicromapBuildInfoEXT* pBuildInfo, VkMicromapBuildSizesInfoEXT* pSizeInfo) {
    static PFN_vkGetMicromapBuildSizesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetMicromapBuildSizesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetMicromapBuildSizesEXT");
    if (pfn) pfn(device, buildType, pBuildInfo, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawClusterHUAWEI(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    static PFN_vkCmdDrawClusterHUAWEI pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawClusterHUAWEI)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawClusterHUAWEI");
    if (pfn) pfn(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawClusterIndirectHUAWEI(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    static PFN_vkCmdDrawClusterIndirectHUAWEI pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawClusterIndirectHUAWEI)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawClusterIndirectHUAWEI");
    if (pfn) pfn(commandBuffer, buffer, offset);
}

VKAPI_ATTR void VKAPI_CALL vkSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority) {
    static PFN_vkSetDeviceMemoryPriorityEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetDeviceMemoryPriorityEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetDeviceMemoryPriorityEXT");
    if (pfn) pfn(device, memory, priority);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDispatchParametersARM(VkCommandBuffer commandBuffer, const VkDispatchParametersARM* pDispatchParameters) {
    static PFN_vkCmdSetDispatchParametersARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDispatchParametersARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDispatchParametersARM");
    if (pfn) pfn(commandBuffer, pDispatchParameters);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetLayoutHostMappingInfoVALVE(VkDevice device, const VkDescriptorSetBindingReferenceVALVE* pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) {
    static PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
    if (pfn) pfn(device, pBindingReference, pHostMapping);
}

VKAPI_ATTR void VKAPI_CALL vkGetDescriptorSetHostMappingVALVE(VkDevice device, VkDescriptorSet descriptorSet, void** ppData) {
    static PFN_vkGetDescriptorSetHostMappingVALVE pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDescriptorSetHostMappingVALVE)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDescriptorSetHostMappingVALVE");
    if (pfn) pfn(device, descriptorSet, ppData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride) {
    static PFN_vkCmdCopyMemoryIndirectNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryIndirectNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryIndirectNV");
    if (pfn) pfn(commandBuffer, copyBufferAddress, copyCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToImageIndirectNV(VkCommandBuffer commandBuffer, VkDeviceAddress copyBufferAddress, uint32_t copyCount, uint32_t stride, VkImage dstImage, VkImageLayout dstImageLayout, const VkImageSubresourceLayers* pImageSubresources) {
    static PFN_vkCmdCopyMemoryToImageIndirectNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryToImageIndirectNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryToImageIndirectNV");
    if (pfn) pfn(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDecompressMemoryNV(VkCommandBuffer commandBuffer, uint32_t decompressRegionCount, const VkDecompressMemoryRegionNV* pDecompressMemoryRegions) {
    static PFN_vkCmdDecompressMemoryNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDecompressMemoryNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDecompressMemoryNV");
    if (pfn) pfn(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDecompressMemoryIndirectCountNV(VkCommandBuffer commandBuffer, VkDeviceAddress indirectCommandsAddress, VkDeviceAddress indirectCommandsCountAddress, uint32_t stride) {
    static PFN_vkCmdDecompressMemoryIndirectCountNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDecompressMemoryIndirectCountNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDecompressMemoryIndirectCountNV");
    if (pfn) pfn(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
}

VKAPI_ATTR void VKAPI_CALL vkGetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo* pCreateInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetPipelineIndirectMemoryRequirementsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineIndirectMemoryRequirementsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineIndirectMemoryRequirementsNV");
    if (pfn) pfn(device, pCreateInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdatePipelineIndirectBufferNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    static PFN_vkCmdUpdatePipelineIndirectBufferNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdUpdatePipelineIndirectBufferNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdUpdatePipelineIndirectBufferNV");
    if (pfn) pfn(commandBuffer, pipelineBindPoint, pipeline);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV* pInfo) {
    static PFN_vkGetPipelineIndirectDeviceAddressNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPipelineIndirectDeviceAddressNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPipelineIndirectDeviceAddressNV");
    return pfn ? pfn(device, pInfo) : (VkDeviceAddress)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    static PFN_vkCmdSetDepthClampEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthClampEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthClampEnableEXT");
    if (pfn) pfn(commandBuffer, depthClampEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    static PFN_vkCmdSetPolygonModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPolygonModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPolygonModeEXT");
    if (pfn) pfn(commandBuffer, polygonMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples) {
    static PFN_vkCmdSetRasterizationSamplesEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRasterizationSamplesEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRasterizationSamplesEXT");
    if (pfn) pfn(commandBuffer, rasterizationSamples);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples, const VkSampleMask* pSampleMask) {
    static PFN_vkCmdSetSampleMaskEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetSampleMaskEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetSampleMaskEXT");
    if (pfn) pfn(commandBuffer, samples, pSampleMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable) {
    static PFN_vkCmdSetAlphaToCoverageEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetAlphaToCoverageEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetAlphaToCoverageEnableEXT");
    if (pfn) pfn(commandBuffer, alphaToCoverageEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    static PFN_vkCmdSetAlphaToOneEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetAlphaToOneEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetAlphaToOneEnableEXT");
    if (pfn) pfn(commandBuffer, alphaToOneEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    static PFN_vkCmdSetLogicOpEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLogicOpEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLogicOpEnableEXT");
    if (pfn) pfn(commandBuffer, logicOpEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkBool32* pColorBlendEnables) {
    static PFN_vkCmdSetColorBlendEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetColorBlendEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetColorBlendEnableEXT");
    if (pfn) pfn(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations) {
    static PFN_vkCmdSetColorBlendEquationEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetColorBlendEquationEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetColorBlendEquationEXT");
    if (pfn) pfn(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorComponentFlags* pColorWriteMasks) {
    static PFN_vkCmdSetColorWriteMaskEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetColorWriteMaskEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetColorWriteMaskEXT");
    if (pfn) pfn(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin) {
    static PFN_vkCmdSetTessellationDomainOriginEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetTessellationDomainOriginEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetTessellationDomainOriginEXT");
    if (pfn) pfn(commandBuffer, domainOrigin);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) {
    static PFN_vkCmdSetRasterizationStreamEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRasterizationStreamEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRasterizationStreamEXT");
    if (pfn) pfn(commandBuffer, rasterizationStream);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    static PFN_vkCmdSetConservativeRasterizationModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetConservativeRasterizationModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetConservativeRasterizationModeEXT");
    if (pfn) pfn(commandBuffer, conservativeRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer, float extraPrimitiveOverestimationSize) {
    static PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    if (pfn) pfn(commandBuffer, extraPrimitiveOverestimationSize);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    static PFN_vkCmdSetDepthClipEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthClipEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthClipEnableEXT");
    if (pfn) pfn(commandBuffer, depthClipEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable) {
    static PFN_vkCmdSetSampleLocationsEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetSampleLocationsEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetSampleLocationsEnableEXT");
    if (pfn) pfn(commandBuffer, sampleLocationsEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced) {
    static PFN_vkCmdSetColorBlendAdvancedEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetColorBlendAdvancedEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetColorBlendAdvancedEXT");
    if (pfn) pfn(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode) {
    static PFN_vkCmdSetProvokingVertexModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetProvokingVertexModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetProvokingVertexModeEXT");
    if (pfn) pfn(commandBuffer, provokingVertexMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer, VkLineRasterizationModeEXT lineRasterizationMode) {
    static PFN_vkCmdSetLineRasterizationModeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineRasterizationModeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineRasterizationModeEXT");
    if (pfn) pfn(commandBuffer, lineRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    static PFN_vkCmdSetLineStippleEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetLineStippleEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetLineStippleEnableEXT");
    if (pfn) pfn(commandBuffer, stippledLineEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) {
    static PFN_vkCmdSetDepthClipNegativeOneToOneEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthClipNegativeOneToOneEXT");
    if (pfn) pfn(commandBuffer, negativeOneToOne);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable) {
    static PFN_vkCmdSetViewportWScalingEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportWScalingEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportWScalingEnableNV");
    if (pfn) pfn(commandBuffer, viewportWScalingEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportSwizzleNV* pViewportSwizzles) {
    static PFN_vkCmdSetViewportSwizzleNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetViewportSwizzleNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetViewportSwizzleNV");
    if (pfn) pfn(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) {
    static PFN_vkCmdSetCoverageToColorEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageToColorEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageToColorEnableNV");
    if (pfn) pfn(commandBuffer, coverageToColorEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation) {
    static PFN_vkCmdSetCoverageToColorLocationNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageToColorLocationNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageToColorLocationNV");
    if (pfn) pfn(commandBuffer, coverageToColorLocation);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer, VkCoverageModulationModeNV coverageModulationMode) {
    static PFN_vkCmdSetCoverageModulationModeNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageModulationModeNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageModulationModeNV");
    if (pfn) pfn(commandBuffer, coverageModulationMode);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable) {
    static PFN_vkCmdSetCoverageModulationTableEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageModulationTableEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageModulationTableEnableNV");
    if (pfn) pfn(commandBuffer, coverageModulationTableEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount, const float* pCoverageModulationTable) {
    static PFN_vkCmdSetCoverageModulationTableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageModulationTableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageModulationTableNV");
    if (pfn) pfn(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable) {
    static PFN_vkCmdSetShadingRateImageEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetShadingRateImageEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetShadingRateImageEnableNV");
    if (pfn) pfn(commandBuffer, shadingRateImageEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer, VkBool32 representativeFragmentTestEnable) {
    static PFN_vkCmdSetRepresentativeFragmentTestEnableNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRepresentativeFragmentTestEnableNV");
    if (pfn) pfn(commandBuffer, representativeFragmentTestEnable);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode) {
    static PFN_vkCmdSetCoverageReductionModeNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetCoverageReductionModeNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetCoverageReductionModeNV");
    if (pfn) pfn(commandBuffer, coverageReductionMode);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateTensorARM(VkDevice device, const VkTensorCreateInfoARM* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkTensorARM* pTensor) {
    static PFN_vkCreateTensorARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateTensorARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateTensorARM");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pTensor) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyTensorARM(VkDevice device, VkTensorARM tensor, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyTensorARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyTensorARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyTensorARM");
    if (pfn) pfn(device, tensor, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateTensorViewARM(VkDevice device, const VkTensorViewCreateInfoARM* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkTensorViewARM* pView) {
    static PFN_vkCreateTensorViewARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateTensorViewARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateTensorViewARM");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pView) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyTensorViewARM(VkDevice device, VkTensorViewARM tensorView, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyTensorViewARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyTensorViewARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyTensorViewARM");
    if (pfn) pfn(device, tensorView, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetTensorMemoryRequirementsARM(VkDevice device, const VkTensorMemoryRequirementsInfoARM* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetTensorMemoryRequirementsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetTensorMemoryRequirementsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetTensorMemoryRequirementsARM");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindTensorMemoryARM(VkDevice device, uint32_t bindInfoCount, const VkBindTensorMemoryInfoARM* pBindInfos) {
    static PFN_vkBindTensorMemoryARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindTensorMemoryARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindTensorMemoryARM");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceTensorMemoryRequirementsARM(VkDevice device, const VkDeviceTensorMemoryRequirementsARM* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDeviceTensorMemoryRequirementsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceTensorMemoryRequirementsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceTensorMemoryRequirementsARM");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyTensorARM(VkCommandBuffer commandBuffer, const VkCopyTensorInfoARM* pCopyTensorInfo) {
    static PFN_vkCmdCopyTensorARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyTensorARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyTensorARM");
    if (pfn) pfn(commandBuffer, pCopyTensorInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalTensorPropertiesARM(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalTensorInfoARM* pExternalTensorInfo, VkExternalTensorPropertiesARM* pExternalTensorProperties) {
    static PFN_vkGetPhysicalDeviceExternalTensorPropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceExternalTensorPropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceExternalTensorPropertiesARM");
    if (pfn) pfn(physicalDevice, pExternalTensorInfo, pExternalTensorProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetTensorOpaqueCaptureDescriptorDataARM(VkDevice device, const VkTensorCaptureDescriptorDataInfoARM* pInfo, void* pData) {
    static PFN_vkGetTensorOpaqueCaptureDescriptorDataARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetTensorOpaqueCaptureDescriptorDataARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetTensorOpaqueCaptureDescriptorDataARM");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetTensorViewOpaqueCaptureDescriptorDataARM(VkDevice device, const VkTensorViewCaptureDescriptorDataInfoARM* pInfo, void* pData) {
    static PFN_vkGetTensorViewOpaqueCaptureDescriptorDataARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetTensorViewOpaqueCaptureDescriptorDataARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetTensorViewOpaqueCaptureDescriptorDataARM");
    return pfn ? pfn(device, pInfo, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule, VkShaderModuleIdentifierEXT* pIdentifier) {
    static PFN_vkGetShaderModuleIdentifierEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetShaderModuleIdentifierEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetShaderModuleIdentifierEXT");
    if (pfn) pfn(device, shaderModule, pIdentifier);
}

VKAPI_ATTR void VKAPI_CALL vkGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, VkShaderModuleIdentifierEXT* pIdentifier) {
    static PFN_vkGetShaderModuleCreateInfoIdentifierEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetShaderModuleCreateInfoIdentifierEXT");
    if (pfn) pfn(device, pCreateInfo, pIdentifier);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount, VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
    return pfn ? pfn(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateOpticalFlowSessionNV(VkDevice device, const VkOpticalFlowSessionCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkOpticalFlowSessionNV* pSession) {
    static PFN_vkCreateOpticalFlowSessionNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateOpticalFlowSessionNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateOpticalFlowSessionNV");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSession) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyOpticalFlowSessionNV(VkDevice device, VkOpticalFlowSessionNV session, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyOpticalFlowSessionNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyOpticalFlowSessionNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyOpticalFlowSessionNV");
    if (pfn) pfn(device, session, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindOpticalFlowSessionImageNV(VkDevice device, VkOpticalFlowSessionNV session, VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view, VkImageLayout layout) {
    static PFN_vkBindOpticalFlowSessionImageNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindOpticalFlowSessionImageNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindOpticalFlowSessionImageNV");
    return pfn ? pfn(device, session, bindingPoint, view, layout) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdOpticalFlowExecuteNV(VkCommandBuffer commandBuffer, VkOpticalFlowSessionNV session, const VkOpticalFlowExecuteInfoNV* pExecuteInfo) {
    static PFN_vkCmdOpticalFlowExecuteNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdOpticalFlowExecuteNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdOpticalFlowExecuteNV");
    if (pfn) pfn(commandBuffer, session, pExecuteInfo);
}

VKAPI_ATTR void VKAPI_CALL vkAntiLagUpdateAMD(VkDevice device, const VkAntiLagDataAMD* pData) {
    static PFN_vkAntiLagUpdateAMD pfn = NULL;
    if (!pfn) pfn = (PFN_vkAntiLagUpdateAMD)vk_icdGetInstanceProcAddr(s_vkInstance, "vkAntiLagUpdateAMD");
    if (pfn) pfn(device, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders) {
    static PFN_vkCreateShadersEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateShadersEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateShadersEXT");
    return pfn ? pfn(device, createInfoCount, pCreateInfos, pAllocator, pShaders) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyShaderEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyShaderEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyShaderEXT");
    if (pfn) pfn(device, shader, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t* pDataSize, void* pData) {
    static PFN_vkGetShaderBinaryDataEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetShaderBinaryDataEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetShaderBinaryDataEXT");
    return pfn ? pfn(device, shader, pDataSize, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages, const VkShaderEXT* pShaders) {
    static PFN_vkCmdBindShadersEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindShadersEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindShadersEXT");
    if (pfn) pfn(commandBuffer, stageCount, pStages, pShaders);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode, const VkDepthClampRangeEXT* pDepthClampRange) {
    static PFN_vkCmdSetDepthClampRangeEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetDepthClampRangeEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetDepthClampRangeEXT");
    if (pfn) pfn(commandBuffer, depthClampMode, pDepthClampRange);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFramebufferTilePropertiesQCOM(VkDevice device, VkFramebuffer framebuffer, uint32_t* pPropertiesCount, VkTilePropertiesQCOM* pProperties) {
    static PFN_vkGetFramebufferTilePropertiesQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetFramebufferTilePropertiesQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetFramebufferTilePropertiesQCOM");
    return pfn ? pfn(device, framebuffer, pPropertiesCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDynamicRenderingTilePropertiesQCOM(VkDevice device, const VkRenderingInfo* pRenderingInfo, VkTilePropertiesQCOM* pProperties) {
    static PFN_vkGetDynamicRenderingTilePropertiesQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDynamicRenderingTilePropertiesQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDynamicRenderingTilePropertiesQCOM");
    return pfn ? pfn(device, pRenderingInfo, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeVectorPropertiesNV* pProperties) {
    static PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkConvertCooperativeVectorMatrixNV(VkDevice device, const VkConvertCooperativeVectorMatrixInfoNV* pInfo) {
    static PFN_vkConvertCooperativeVectorMatrixNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkConvertCooperativeVectorMatrixNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkConvertCooperativeVectorMatrixNV");
    return pfn ? pfn(device, pInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkConvertCooperativeVectorMatrixInfoNV* pInfos) {
    static PFN_vkCmdConvertCooperativeVectorMatrixNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdConvertCooperativeVectorMatrixNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdConvertCooperativeVectorMatrixNV");
    if (pfn) pfn(commandBuffer, infoCount, pInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo) {
    static PFN_vkSetLatencySleepModeNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetLatencySleepModeNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetLatencySleepModeNV");
    return pfn ? pfn(device, swapchain, pSleepModeInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
    static PFN_vkLatencySleepNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkLatencySleepNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkLatencySleepNV");
    return pfn ? pfn(device, swapchain, pSleepInfo) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkSetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    static PFN_vkSetLatencyMarkerNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkSetLatencyMarkerNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkSetLatencyMarkerNV");
    if (pfn) pfn(device, swapchain, pLatencyMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
    static PFN_vkGetLatencyTimingsNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetLatencyTimingsNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetLatencyTimingsNV");
    if (pfn) pfn(device, swapchain, pLatencyMarkerInfo);
}

VKAPI_ATTR void VKAPI_CALL vkQueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
    static PFN_vkQueueNotifyOutOfBandNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkQueueNotifyOutOfBandNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkQueueNotifyOutOfBandNV");
    if (pfn) pfn(queue, pQueueTypeInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDataGraphPipelinesARM(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkDataGraphPipelineCreateInfoARM* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    static PFN_vkCreateDataGraphPipelinesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDataGraphPipelinesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDataGraphPipelinesARM");
    return pfn ? pfn(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDataGraphPipelineSessionARM(VkDevice device, const VkDataGraphPipelineSessionCreateInfoARM* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDataGraphPipelineSessionARM* pSession) {
    static PFN_vkCreateDataGraphPipelineSessionARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateDataGraphPipelineSessionARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateDataGraphPipelineSessionARM");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pSession) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDataGraphPipelineSessionBindPointRequirementsARM(VkDevice device, const VkDataGraphPipelineSessionBindPointRequirementsInfoARM* pInfo, uint32_t* pBindPointRequirementCount, VkDataGraphPipelineSessionBindPointRequirementARM* pBindPointRequirements) {
    static PFN_vkGetDataGraphPipelineSessionBindPointRequirementsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDataGraphPipelineSessionBindPointRequirementsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDataGraphPipelineSessionBindPointRequirementsARM");
    return pfn ? pfn(device, pInfo, pBindPointRequirementCount, pBindPointRequirements) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetDataGraphPipelineSessionMemoryRequirementsARM(VkDevice device, const VkDataGraphPipelineSessionMemoryRequirementsInfoARM* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetDataGraphPipelineSessionMemoryRequirementsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDataGraphPipelineSessionMemoryRequirementsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDataGraphPipelineSessionMemoryRequirementsARM");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindDataGraphPipelineSessionMemoryARM(VkDevice device, uint32_t bindInfoCount, const VkBindDataGraphPipelineSessionMemoryInfoARM* pBindInfos) {
    static PFN_vkBindDataGraphPipelineSessionMemoryARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkBindDataGraphPipelineSessionMemoryARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBindDataGraphPipelineSessionMemoryARM");
    return pfn ? pfn(device, bindInfoCount, pBindInfos) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDataGraphPipelineSessionARM(VkDevice device, VkDataGraphPipelineSessionARM session, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyDataGraphPipelineSessionARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyDataGraphPipelineSessionARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyDataGraphPipelineSessionARM");
    if (pfn) pfn(device, session, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchDataGraphARM(VkCommandBuffer commandBuffer, VkDataGraphPipelineSessionARM session, const VkDataGraphPipelineDispatchInfoARM* pInfo) {
    static PFN_vkCmdDispatchDataGraphARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDispatchDataGraphARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDispatchDataGraphARM");
    if (pfn) pfn(commandBuffer, session, pInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDataGraphPipelineAvailablePropertiesARM(VkDevice device, const VkDataGraphPipelineInfoARM* pPipelineInfo, uint32_t* pPropertiesCount, VkDataGraphPipelinePropertyARM* pProperties) {
    static PFN_vkGetDataGraphPipelineAvailablePropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDataGraphPipelineAvailablePropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDataGraphPipelineAvailablePropertiesARM");
    return pfn ? pfn(device, pPipelineInfo, pPropertiesCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDataGraphPipelinePropertiesARM(VkDevice device, const VkDataGraphPipelineInfoARM* pPipelineInfo, uint32_t propertiesCount, VkDataGraphPipelinePropertyQueryResultARM* pProperties) {
    static PFN_vkGetDataGraphPipelinePropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDataGraphPipelinePropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDataGraphPipelinePropertiesARM");
    return pfn ? pfn(device, pPipelineInfo, propertiesCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pQueueFamilyDataGraphPropertyCount, VkQueueFamilyDataGraphPropertiesARM* pQueueFamilyDataGraphProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, pQueueFamilyDataGraphPropertyCount, pQueueFamilyDataGraphProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceQueueFamilyDataGraphProcessingEngineInfoARM* pQueueFamilyDataGraphProcessingEngineInfo, VkQueueFamilyDataGraphProcessingEnginePropertiesARM* pQueueFamilyDataGraphProcessingEngineProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyDataGraphProcessingEnginePropertiesARM");
    if (pfn) pfn(physicalDevice, pQueueFamilyDataGraphProcessingEngineInfo, pQueueFamilyDataGraphProcessingEngineProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const VkQueueFamilyDataGraphPropertiesARM* pQueueFamilyDataGraphProperties, VkBaseOutStructure* pProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, pQueueFamilyDataGraphProperties, pProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask) {
    static PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
    if (pfn) pfn(commandBuffer, aspectMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindTileMemoryQCOM(VkCommandBuffer commandBuffer, const VkTileMemoryBindInfoQCOM* pTileMemoryBindInfo) {
    static PFN_vkCmdBindTileMemoryQCOM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBindTileMemoryQCOM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBindTileMemoryQCOM");
    if (pfn) pfn(commandBuffer, pTileMemoryBindInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDecompressMemoryEXT(VkCommandBuffer commandBuffer, const VkDecompressMemoryInfoEXT* pDecompressMemoryInfoEXT) {
    static PFN_vkCmdDecompressMemoryEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDecompressMemoryEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDecompressMemoryEXT");
    if (pfn) pfn(commandBuffer, pDecompressMemoryInfoEXT);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDecompressMemoryIndirectCountEXT(VkCommandBuffer commandBuffer, VkMemoryDecompressionMethodFlagsEXT decompressionMethod, VkDeviceAddress indirectCommandsAddress, VkDeviceAddress indirectCommandsCountAddress, uint32_t maxDecompressionCount, uint32_t stride) {
    static PFN_vkCmdDecompressMemoryIndirectCountEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDecompressMemoryIndirectCountEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDecompressMemoryIndirectCountEXT");
    if (pfn) pfn(commandBuffer, decompressionMethod, indirectCommandsAddress, indirectCommandsCountAddress, maxDecompressionCount, stride);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateExternalComputeQueueNV(VkDevice device, const VkExternalComputeQueueCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkExternalComputeQueueNV* pExternalQueue) {
    static PFN_vkCreateExternalComputeQueueNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateExternalComputeQueueNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateExternalComputeQueueNV");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pExternalQueue) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyExternalComputeQueueNV(VkDevice device, VkExternalComputeQueueNV externalQueue, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyExternalComputeQueueNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyExternalComputeQueueNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyExternalComputeQueueNV");
    if (pfn) pfn(device, externalQueue, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetExternalComputeQueueDataNV(VkExternalComputeQueueNV externalQueue, VkExternalComputeQueueDataParamsNV* params, void* pData) {
    static PFN_vkGetExternalComputeQueueDataNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetExternalComputeQueueDataNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetExternalComputeQueueDataNV");
    if (pfn) pfn(externalQueue, params, pData);
}

VKAPI_ATTR void VKAPI_CALL vkGetClusterAccelerationStructureBuildSizesNV(VkDevice device, const VkClusterAccelerationStructureInputInfoNV* pInfo, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    static PFN_vkGetClusterAccelerationStructureBuildSizesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetClusterAccelerationStructureBuildSizesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetClusterAccelerationStructureBuildSizesNV");
    if (pfn) pfn(device, pInfo, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildClusterAccelerationStructureIndirectNV(VkCommandBuffer commandBuffer, const VkClusterAccelerationStructureCommandsInfoNV* pCommandInfos) {
    static PFN_vkCmdBuildClusterAccelerationStructureIndirectNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildClusterAccelerationStructureIndirectNV");
    if (pfn) pfn(commandBuffer, pCommandInfos);
}

VKAPI_ATTR void VKAPI_CALL vkGetPartitionedAccelerationStructuresBuildSizesNV(VkDevice device, const VkPartitionedAccelerationStructureInstancesInputNV* pInfo, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    static PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPartitionedAccelerationStructuresBuildSizesNV");
    if (pfn) pfn(device, pInfo, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildPartitionedAccelerationStructuresNV(VkCommandBuffer commandBuffer, const VkBuildPartitionedAccelerationStructureInfoNV* pBuildInfo) {
    static PFN_vkCmdBuildPartitionedAccelerationStructuresNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildPartitionedAccelerationStructuresNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildPartitionedAccelerationStructuresNV");
    if (pfn) pfn(commandBuffer, pBuildInfo);
}

VKAPI_ATTR void VKAPI_CALL vkGetGeneratedCommandsMemoryRequirementsEXT(VkDevice device, const VkGeneratedCommandsMemoryRequirementsInfoEXT* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    static PFN_vkGetGeneratedCommandsMemoryRequirementsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetGeneratedCommandsMemoryRequirementsEXT");
    if (pfn) pfn(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo, VkCommandBuffer stateCommandBuffer) {
    static PFN_vkCmdPreprocessGeneratedCommandsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdPreprocessGeneratedCommandsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdPreprocessGeneratedCommandsEXT");
    if (pfn) pfn(commandBuffer, pGeneratedCommandsInfo, stateCommandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo) {
    static PFN_vkCmdExecuteGeneratedCommandsEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdExecuteGeneratedCommandsEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdExecuteGeneratedCommandsEXT");
    if (pfn) pfn(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout) {
    static PFN_vkCreateIndirectCommandsLayoutEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateIndirectCommandsLayoutEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateIndirectCommandsLayoutEXT");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pIndirectCommandsLayout) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyIndirectCommandsLayoutEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyIndirectCommandsLayoutEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyIndirectCommandsLayoutEXT");
    if (pfn) pfn(device, indirectCommandsLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectExecutionSetEXT* pIndirectExecutionSet) {
    static PFN_vkCreateIndirectExecutionSetEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateIndirectExecutionSetEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateIndirectExecutionSetEXT");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pIndirectExecutionSet) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyIndirectExecutionSetEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyIndirectExecutionSetEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyIndirectExecutionSetEXT");
    if (pfn) pfn(device, indirectExecutionSet, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateIndirectExecutionSetPipelineEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetPipelineEXT* pExecutionSetWrites) {
    static PFN_vkUpdateIndirectExecutionSetPipelineEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateIndirectExecutionSetPipelineEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateIndirectExecutionSetPipelineEXT");
    if (pfn) pfn(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateIndirectExecutionSetShaderEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet, uint32_t executionSetWriteCount, const VkWriteIndirectExecutionSetShaderEXT* pExecutionSetWrites) {
    static PFN_vkUpdateIndirectExecutionSetShaderEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkUpdateIndirectExecutionSetShaderEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkUpdateIndirectExecutionSetShaderEXT");
    if (pfn) pfn(device, indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties) {
    static PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
    return pfn ? pfn(physicalDevice, pPropertyCount, pProperties) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterARM* pCounters, VkPerformanceCounterDescriptionARM* pCounterDescriptions) {
    static PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceCountersByRegionARM");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM(VkPhysicalDevice physicalDevice, uint32_t* pDescriptionCount, VkShaderInstrumentationMetricDescriptionARM* pDescriptions) {
    static PFN_vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkEnumeratePhysicalDeviceShaderInstrumentationMetricsARM");
    return pfn ? pfn(physicalDevice, pDescriptionCount, pDescriptions) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderInstrumentationARM(VkDevice device, const VkShaderInstrumentationCreateInfoARM* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderInstrumentationARM* pInstrumentation) {
    static PFN_vkCreateShaderInstrumentationARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateShaderInstrumentationARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateShaderInstrumentationARM");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pInstrumentation) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderInstrumentationARM(VkDevice device, VkShaderInstrumentationARM instrumentation, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyShaderInstrumentationARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyShaderInstrumentationARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyShaderInstrumentationARM");
    if (pfn) pfn(device, instrumentation, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginShaderInstrumentationARM(VkCommandBuffer commandBuffer, VkShaderInstrumentationARM instrumentation) {
    static PFN_vkCmdBeginShaderInstrumentationARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginShaderInstrumentationARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginShaderInstrumentationARM");
    if (pfn) pfn(commandBuffer, instrumentation);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndShaderInstrumentationARM(VkCommandBuffer commandBuffer) {
    static PFN_vkCmdEndShaderInstrumentationARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndShaderInstrumentationARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndShaderInstrumentationARM");
    if (pfn) pfn(commandBuffer);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetShaderInstrumentationValuesARM(VkDevice device, VkShaderInstrumentationARM instrumentation, uint32_t* pMetricBlockCount, void* pMetricValues, VkShaderInstrumentationValuesFlagsARM flags) {
    static PFN_vkGetShaderInstrumentationValuesARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetShaderInstrumentationValuesARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetShaderInstrumentationValuesARM");
    return pfn ? pfn(device, instrumentation, pMetricBlockCount, pMetricValues, flags) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkClearShaderInstrumentationMetricsARM(VkDevice device, VkShaderInstrumentationARM instrumentation) {
    static PFN_vkClearShaderInstrumentationMetricsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkClearShaderInstrumentationMetricsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkClearShaderInstrumentationMetricsARM");
    if (pfn) pfn(device, instrumentation);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRendering2EXT(VkCommandBuffer commandBuffer, const VkRenderingEndInfoKHR* pRenderingEndInfo) {
    static PFN_vkCmdEndRendering2EXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdEndRendering2EXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdEndRendering2EXT");
    if (pfn) pfn(commandBuffer, pRenderingEndInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginCustomResolveEXT(VkCommandBuffer commandBuffer, const VkBeginCustomResolveInfoEXT* pBeginCustomResolveInfo) {
    static PFN_vkCmdBeginCustomResolveEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBeginCustomResolveEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBeginCustomResolveEXT");
    if (pfn) pfn(commandBuffer, pBeginCustomResolveInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const VkQueueFamilyDataGraphPropertiesARM* pQueueFamilyDataGraphProperties, const VkDataGraphOpticalFlowImageFormatInfoARM* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount, VkDataGraphOpticalFlowImageFormatPropertiesARM* pImageFormatProperties) {
    static PFN_vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM");
    return pfn ? pfn(physicalDevice, queueFamilyIndex, pQueueFamilyDataGraphProperties, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetComputeOccupancyPriorityNV(VkCommandBuffer commandBuffer, const VkComputeOccupancyPriorityParametersNV* pParameters) {
    static PFN_vkCmdSetComputeOccupancyPriorityNV pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetComputeOccupancyPriorityNV)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetComputeOccupancyPriorityNV");
    if (pfn) pfn(commandBuffer, pParameters);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetPrimitiveRestartIndexEXT(VkCommandBuffer commandBuffer, uint32_t primitiveRestartIndex) {
    static PFN_vkCmdSetPrimitiveRestartIndexEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetPrimitiveRestartIndexEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetPrimitiveRestartIndexEXT");
    if (pfn) pfn(commandBuffer, primitiveRestartIndex);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure) {
    static PFN_vkCreateAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateAccelerationStructureKHR");
    return pfn ? pfn(device, pCreateInfo, pAllocator, pAccelerationStructure) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) {
    static PFN_vkDestroyAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkDestroyAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkDestroyAccelerationStructureKHR");
    if (pfn) pfn(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    static PFN_vkCmdBuildAccelerationStructuresKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildAccelerationStructuresKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildAccelerationStructuresKHR");
    if (pfn) pfn(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress* pIndirectDeviceAddresses, const uint32_t* pIndirectStrides, const uint32_t* const* ppMaxPrimitiveCounts) {
    static PFN_vkCmdBuildAccelerationStructuresIndirectKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdBuildAccelerationStructuresIndirectKHR");
    if (pfn) pfn(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    static PFN_vkBuildAccelerationStructuresKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkBuildAccelerationStructuresKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkBuildAccelerationStructuresKHR");
    return pfn ? pfn(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    static PFN_vkCopyAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyAccelerationStructureKHR");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    static PFN_vkCopyAccelerationStructureToMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyAccelerationStructureToMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyAccelerationStructureToMemoryKHR");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    static PFN_vkCopyMemoryToAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCopyMemoryToAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCopyMemoryToAccelerationStructureKHR");
    return pfn ? pfn(device, deferredOperation, pInfo) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, size_t dataSize, void* pData, size_t stride) {
    static PFN_vkWriteAccelerationStructuresPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkWriteAccelerationStructuresPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkWriteAccelerationStructuresPropertiesKHR");
    return pfn ? pfn(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) {
    static PFN_vkCmdCopyAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyAccelerationStructureKHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    static PFN_vkCmdCopyAccelerationStructureToMemoryKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyAccelerationStructureToMemoryKHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    static PFN_vkCmdCopyMemoryToAccelerationStructureKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdCopyMemoryToAccelerationStructureKHR");
    if (pfn) pfn(commandBuffer, pInfo);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    static PFN_vkGetAccelerationStructureDeviceAddressKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetAccelerationStructureDeviceAddressKHR");
    return pfn ? pfn(device, pInfo) : (VkDeviceAddress)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    if (pfn) pfn(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceAccelerationStructureCompatibilityKHR(VkDevice device, const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) {
    static PFN_vkGetDeviceAccelerationStructureCompatibilityKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    if (pfn) pfn(device, pVersionInfo, pCompatibility);
}

VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo, const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    static PFN_vkGetAccelerationStructureBuildSizesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetAccelerationStructureBuildSizesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetAccelerationStructureBuildSizesKHR");
    if (pfn) pfn(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) {
    static PFN_vkCmdTraceRaysKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdTraceRaysKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdTraceRaysKHR");
    if (pfn) pfn(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    static PFN_vkCreateRayTracingPipelinesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCreateRayTracingPipelinesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCreateRayTracingPipelinesKHR");
    return pfn ? pfn(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines) : (VkResult)0;
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) {
    static PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    return pfn ? pfn(device, pipeline, firstGroup, groupCount, dataSize, pData) : (VkResult)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) {
    static PFN_vkCmdTraceRaysIndirectKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdTraceRaysIndirectKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdTraceRaysIndirectKHR");
    if (pfn) pfn(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}

VKAPI_ATTR VkDeviceSize VKAPI_CALL vkGetRayTracingShaderGroupStackSizeKHR(VkDevice device, VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader) {
    static PFN_vkGetRayTracingShaderGroupStackSizeKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkGetRayTracingShaderGroupStackSizeKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkGetRayTracingShaderGroupStackSizeKHR");
    return pfn ? pfn(device, pipeline, group, groupShader) : (VkDeviceSize)0;
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) {
    static PFN_vkCmdSetRayTracingPipelineStackSizeKHR pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdSetRayTracingPipelineStackSizeKHR");
    if (pfn) pfn(commandBuffer, pipelineStackSize);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    static PFN_vkCmdDrawMeshTasksEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksEXT");
    if (pfn) pfn(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    static PFN_vkCmdDrawMeshTasksIndirectEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirectEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirectEXT");
    if (pfn) pfn(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
    static PFN_vkCmdDrawMeshTasksIndirectCountEXT pfn = NULL;
    if (!pfn) pfn = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)vk_icdGetInstanceProcAddr(s_vkInstance, "vkCmdDrawMeshTasksIndirectCountEXT");
    if (pfn) pfn(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

