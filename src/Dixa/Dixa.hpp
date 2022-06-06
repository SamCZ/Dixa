#pragma once

#include <cstdint>
#include <DXGI.h>
#include <D3D11.h>

#ifndef ENUM_FLAGS
#define ENUM_FLAGS(enum_name, num_type) \
    inline enum_name operator|(enum_name a, enum_name b)\
    {\
        return static_cast<enum_name>(static_cast<num_type>(a) | static_cast<num_type>(b));\
    }\
    inline bool operator &(enum_name a, enum_name b)\
    {\
        return (static_cast<num_type>(a) & static_cast<num_type>(b)) != 0;\
    }\
    inline enum_name& operator |=(enum_name& a, enum_name b)\
    {\
        return a = a | b;\
    }
#endif

namespace Dixa
{
	enum class BufferUsage : uint8_t
	{
		Default = D3D11_USAGE_DEFAULT,
		Immutable = D3D11_USAGE_IMMUTABLE,
		Dynamic = D3D11_USAGE_DYNAMIC,
		Staging = D3D11_USAGE_STAGING
	};

	enum class BindFlag
	{
		VertexBuffer = D3D11_BIND_VERTEX_BUFFER,
		IndexBuffer = D3D11_BIND_INDEX_BUFFER,
		ConstantBuffer = D3D11_BIND_CONSTANT_BUFFER,
		ShaderResource = D3D11_BIND_SHADER_RESOURCE,
		StreamOutput = D3D11_BIND_STREAM_OUTPUT,
		RenderTarget = D3D11_BIND_RENDER_TARGET,
		DepthStencil = D3D11_BIND_DEPTH_STENCIL,
		UnorderedAccess = D3D11_BIND_UNORDERED_ACCESS,
		Decoder = D3D11_BIND_DECODER,
		VideoEncoder = D3D11_BIND_VIDEO_ENCODER
	};
	ENUM_FLAGS(BindFlag, int32_t);

	enum class CpuAccessFlag
	{
		None = 0,
		Write = D3D11_CPU_ACCESS_WRITE,
		Read = D3D11_CPU_ACCESS_READ
	};
	ENUM_FLAGS(CpuAccessFlag, int32_t);

	enum class ResourceMiscFlag
	{
		None = 0,
		// Enables MIP map generation by using ID3D11DeviceContext::GenerateMips on a texture resource. The resource must be created with the bind flags that specify that the resource is a render target and a shader resource.
		GenerateMips = D3D11_RESOURCE_MISC_GENERATE_MIPS,
		// Enables resource data sharing between two or more Direct3D devices. The only resources that can be shared are 2D non-mipmapped textures.
		Shared = D3D11_RESOURCE_MISC_SHARED,
		// Sets a resource to be a cube texture created from a Texture2DArray that contains 6 textures.
		TextureCube = D3D11_RESOURCE_MISC_TEXTURECUBE,
		// Enables instancing of GPU-generated content.
		DrawIndirectArgs = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
		// Enables a resource as a byte address buffer.
		BufferAllowRawViews = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
		// Enables a resource as a structured buffer.
		BufferStructured = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		// Enables a resource with MIP map clamping for use with ID3D11DeviceContext::SetResourceMinLOD.
		ResourceClamp = D3D11_RESOURCE_MISC_RESOURCE_CLAMP,
		// Enables the resource to be synchronized by using the IDXGIKeyedMutex::AcquireSync and
		// IDXGIKeyedMutex::ReleaseSync APIs.
		// The following Direct3D 11 resource creation APIs, that take D3D11_RESOURCE_MISC_FLAG parameters, have been extended to support the new flag.
		SharedKeyedMutex = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX,
		// Enables a resource compatible with GDI. You must set the D3D11_RESOURCE_MISC_GDI_COMPATIBLE flag on surfaces that you use with GDI. Setting the D3D11_RESOURCE_MISC_GDI_COMPATIBLE flag allows GDI rendering on the surface via IDXGISurface1::GetDC.
		GdiCompatible = D3D11_RESOURCE_MISC_GDI_COMPATIBLE,
		// Set this flag to enable the use of NT HANDLE values when you create a shared resource. By enabling this flag, you deprecate the use of existing HANDLE values.
		SharedNTHandle = D3D11_RESOURCE_MISC_SHARED_NTHANDLE,
		// Set this flag to indicate that the resource might contain protected content; therefore, the operating system should use the resource only when the driver and hardware support content protection. If the driver and hardware do not support content protection and you try to create a resource with this flag, the resource creation fails.
		RestrictedContent = D3D11_RESOURCE_MISC_RESTRICTED_CONTENT,
		// Set this flag to indicate that the operating system restricts access to the shared surface. You can use this flag together with the D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER flag and only when you create a shared surface. The process that creates the shared resource can always open the shared resource.
		RestrictSharedResource = D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE,
		// Set this flag to indicate that the driver restricts access to the shared surface. You can use this flag in conjunction with the D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE flag and only when you create a shared surface. The process that creates the shared resource can always open the shared resource.
		SharedResourceDriver = D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER,
		// Set this flag to indicate that the resource is guarded. Such a resource is returned by the IDCompositionSurface::BeginDraw (DirectComposition) and ISurfaceImageSourceNative::BeginDraw (Windows Runtime) APIs. For these APIs, you provide a region of interest (ROI) on a surface to update. This surface isn't compatible with multiple render targets (MRT).
		Guarded = D3D11_RESOURCE_MISC_GUARDED,
		// Set this flag to indicate that the resource is a tile pool.
		TilePool = D3D11_RESOURCE_MISC_TILE_POOL,
		// Set this flag to indicate that the resource is a tiled resource.
		Tiled = D3D11_RESOURCE_MISC_TILED,
		// Set this flag to indicate that the resource should be created such that it will be protected by the hardware. Resource creation will fail if hardware content protection is not supported.
		HWProtected = D3D11_RESOURCE_MISC_HW_PROTECTED
	};
	ENUM_FLAGS(ResourceMiscFlag, int32_t);

	enum class MapFlag
	{
		Read = D3D11_MAP_READ,
		Write = D3D11_MAP_WRITE,
		ReadWrite = D3D11_MAP_READ_WRITE,
		WriteDiscard = D3D11_MAP_WRITE_DISCARD,
		WriteNoOverWrite = D3D11_MAP_WRITE_NO_OVERWRITE
	};

	struct BufferDesc
	{
		uint32_t Size;
		uint32_t Stride;
	};

	struct BufferView
	{
		ID3D11ShaderResourceView* View;
		D3D11_SHADER_RESOURCE_VIEW_DESC Desc;
	};

	struct Buffer
	{
		ID3D11Buffer* Buffer;
		BufferView* DefaultView;
	};

	class RenderInterface
	{
	public:
		Buffer* CreateBuffer(uint32_t sizeInBytes, BindFlag bindFlags, BufferUsage usage = BufferUsage::Default, CpuAccessFlag cpuAccessFlags = CpuAccessFlag::None, ResourceMiscFlag micsFlags = ResourceMiscFlag::None, uint32_t structureByteStride = 0);
	};
}