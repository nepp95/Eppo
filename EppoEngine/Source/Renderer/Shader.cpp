#include "pch.h"
#include "Renderer/Shader.h"

#include "Renderer/DeviceManager.h"

#if defined(EP_PLATFORM_WINDOWS)
#include <atlbase.h>
//#include <d3d12shader.h>
#else
#include <dxc/WinAdapter.h>
#endif

#include <dxc/dxcapi.h>

namespace Eppo
{
	Shader::Shader(ShaderSpecification spec)
		: m_Specification(std::move(spec))
	{
		Log::Info("Loading shader '{}'", m_Specification.Name);

		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		const auto bytes = CompileOrGetCache();

		if (m_Specification.IsCompute)
		{
			EP_ASSERT(false);
		}
		else
		{
			nvrhi::ShaderDesc shaderDesc{
				.shaderType = nvrhi::ShaderType::Vertex,
				.entryName = "Main",
			};

			m_VertexShader = device->createShader(shaderDesc, bytes.at(nvrhi::ShaderType::Vertex).data(), bytes.at(nvrhi::ShaderType::Vertex).size());
			shaderDesc.setShaderType(nvrhi::ShaderType::Pixel);
			m_PixelShader = device->createShader(shaderDesc, bytes.at(nvrhi::ShaderType::Pixel).data(), bytes.at(nvrhi::ShaderType::Pixel).size());
		}
	}

	auto Shader::CompileOrGetCache() -> std::unordered_map<nvrhi::ShaderType, std::vector<char>>
	{
		const auto& dm = DeviceManager::Get();
		const auto device = dm->GetDevice();

		std::unordered_map<nvrhi::ShaderType, std::vector<char>> bytes;

		if (m_Specification.IsCompute)
		{
			EP_ASSERT(false);
		}
		else
		{
			const std::filesystem::path cacheDir = FS::GetShaderCacheDirectory();
			const std::filesystem::path vertPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.vert", m_Specification.Name);
			const std::filesystem::path pixelPath = FS::GetResourcesDirectory() / "Shaders" / std::format("{}.frag", m_Specification.Name);
			const std::filesystem::path vertSpvPath = FS::GetShaderCacheDirectory() / std::format("{}.vert.spv", m_Specification.Name);
			const std::filesystem::path pixelSpvPath = FS::GetShaderCacheDirectory() / std::format("{}.frag.spv", m_Specification.Name);
			const std::filesystem::path vertDxilPath = FS::GetShaderCacheDirectory() / std::format("{}.vert.dxil", m_Specification.Name);
			const std::filesystem::path pixelDxilPath = FS::GetShaderCacheDirectory() / std::format("{}.frag.dxil", m_Specification.Name);
			const std::filesystem::path vertCacheHashFile = FS::GetShaderCacheDirectory() / std::format("{}.vert.hash", m_Specification.Name);
			const std::filesystem::path pixelCacheHashFile = FS::GetShaderCacheDirectory() / std::format("{}.frag.hash", m_Specification.Name);
			const auto vertSource = FS::ReadText(vertPath);
			const auto pixelSource = FS::ReadText(pixelPath);

			bool invalidate = true;
			if (FS::Exists(vertSpvPath) && FS::Exists(vertDxilPath) && FS::Exists(vertCacheHashFile))
			{
				std::string vertHash = std::to_string(Hash::GenerateFnv(vertSource));
				std::string pixelHash = std::to_string(Hash::GenerateFnv(pixelSource));
				std::string vertCacheHash = FS::ReadText(vertCacheHashFile);
				std::string pixelCacheHash = FS::ReadText(pixelCacheHashFile);

				if (vertHash == vertCacheHash && pixelHash == pixelCacheHash)
					invalidate = false;
			}
			
			if (!invalidate)
			{
				Log::Info("Loading shader cache for '{}'", m_Specification.Name);

				std::vector<char> vertBytes;
				std::vector<char> pixelBytes;

				switch (dm->GetParams().API)
				{
					case RendererAPI::DX11:
					case RendererAPI::DX12:
					{
						bytes[nvrhi::ShaderType::Vertex] = FS::ReadBytes(vertDxilPath);
						bytes[nvrhi::ShaderType::Pixel] = FS::ReadBytes(pixelDxilPath);
						break;
					}

					case RendererAPI::Vulkan:
					{
						bytes[nvrhi::ShaderType::Vertex] = FS::ReadBytes(vertSpvPath);
						bytes[nvrhi::ShaderType::Pixel] = FS::ReadBytes(pixelSpvPath);
						break;
					}
				}
			}
			else
			{
				Log::Info("Compiling shader '{}'", m_Specification.Name);

				// Compile
				Compile(bytes, nvrhi::ShaderType::Vertex, vertSource);
				Compile(bytes, nvrhi::ShaderType::Pixel, pixelSource);

				// Save hash
				const std::string vertHash = std::to_string(Hash::GenerateFnv(vertSource));
				FS::WriteText(vertCacheHashFile, vertHash, true);
				const std::string pixelHash = std::to_string(Hash::GenerateFnv(pixelSource));
				FS::WriteText(pixelCacheHashFile, pixelHash, true);
			}
		}

		return bytes;
	}

	auto Shader::Compile(std::unordered_map<nvrhi::ShaderType, std::vector<char>>& bytes, const nvrhi::ShaderType type, const std::string& source) -> void
	{
		// Create compiler
		CComPtr<IDxcUtils> utils;
		CComPtr<IDxcCompiler3> compiler;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		// Create include handler
		CComPtr<IDxcIncludeHandler> includeHandler;
		utils->CreateDefaultIncludeHandler(&includeHandler);

		// Command line args for the compiler
		const std::wstring vertPath = std::filesystem::path(FS::GetResourcesDirectory() / "Shaders" / std::format("{}.vert", m_Specification.Name)).wstring();
		const std::wstring pixelPath = std::filesystem::path(FS::GetResourcesDirectory() / "Shaders" / std::format("{}.frag", m_Specification.Name)).wstring();
		const std::wstring vertSpvPath = std::filesystem::path(FS::GetShaderCacheDirectory() / std::format("{}.vert.spv", m_Specification.Name)).wstring();
		const std::wstring pixelSpvPath = std::filesystem::path(FS::GetShaderCacheDirectory() / std::format("{}.frag.spv", m_Specification.Name)).wstring();
		const std::wstring vertDxilPath = std::filesystem::path(FS::GetShaderCacheDirectory() / std::format("{}.vert.dxil", m_Specification.Name)).wstring();
		const std::wstring pixelDxilPath = std::filesystem::path(FS::GetShaderCacheDirectory() / std::format("{}.frag.dxil", m_Specification.Name)).wstring();

		const bool isVertex = type == nvrhi::ShaderType::Vertex ? true : false;
		LPCWSTR argsSpv[] = {
			L"-E", L"Main",
			L"-T", isVertex ? L"vs_6_0" : L"ps_6_0",
			L"-spirv", L"-fvk-t-shift", L"0", L"0", L"-fvk-s-shift", L"128", L"0", L"-fvk-b-shift", L"256", L"0", L"-fvk-u-shift", L"384", L"0",
			isVertex ? vertPath.c_str() : pixelPath.c_str(),
			L"-Fo", isVertex ? vertSpvPath.c_str() : pixelSpvPath.c_str()
		};

		LPCWSTR argsDxil[] = {
			L"-E", L"Main",
			L"-T", isVertex ? L"vs_6_0" : L"ps_6_0",
			isVertex ? vertPath.c_str() : pixelPath.c_str(),
			L"-Fo", isVertex ? vertDxilPath.c_str() : pixelDxilPath.c_str()
		};

		DxcBuffer srcBuffer{
			.Ptr = source.c_str(),
			.Size = source.size(),
			.Encoding = DXC_CP_UTF8,
		};

		for (uint32_t i = 0; i < 2; i++)
		{
			// Compile shader
			CComPtr<IDxcResult> result;

			if (i == 0)
				compiler->Compile(&srcBuffer, argsSpv, _countof(argsSpv), includeHandler, IID_PPV_ARGS(&result));
			else
				compiler->Compile(&srcBuffer, argsDxil, _countof(argsDxil), includeHandler, IID_PPV_ARGS(&result));

			CComPtr<IDxcBlobUtf8> errors = nullptr;
			result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

			if (errors != nullptr && errors->GetStringLength() != 0)
				Log::Error("Failed to compile with errors: \n{}", errors->GetStringPointer());

			HRESULT status;
			result->GetStatus(&status);
			if (FAILED(status))
			{
				Log::Error("Stopped compiling due to errors!");
				EP_ASSERT(false);
				return;
			}

			// Save binary
			CComPtr<IDxcBlob> binary = nullptr;
			CComPtr<IDxcBlobWide> binaryName = nullptr;
			result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&binary), &binaryName);
			if (binary != nullptr)
			{
				if (isVertex)
				{
					const char* pBinary = static_cast<const char*>(binary->GetBufferPointer());
					bytes[nvrhi::ShaderType::Vertex] = std::vector<char>(pBinary, pBinary + binary->GetBufferSize());

					if (i == 0) // Spv
						FS::WriteBytes(vertSpvPath, bytes.at(nvrhi::ShaderType::Vertex), true);
					else // Dxil
						FS::WriteBytes(vertDxilPath, bytes.at(nvrhi::ShaderType::Pixel), true);
				}
				else
				{
					const char* pBinary = static_cast<const char*>(binary->GetBufferPointer());
					bytes[nvrhi::ShaderType::Pixel] = std::vector<char>(pBinary, pBinary + binary->GetBufferSize());

					if (i == 0) // Spv
						FS::WriteBytes(pixelSpvPath, bytes.at(nvrhi::ShaderType::Vertex), true);
					else // Dxil
						FS::WriteBytes(pixelDxilPath, bytes.at(nvrhi::ShaderType::Pixel), true);
				}
			}
		}
	}
}