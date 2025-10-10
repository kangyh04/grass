#include "BaseApp.h"
#include "GeometryGenerator.h"

struct Bone
{
	XMFLOAT3 Position;
	XMFLOAT4 Rotation;
	XMFLOAT3 Velocity;
	float Mass;
	float Length;
	int ParentIndex;
};

class GrassApp : public BaseApp
{
public:
	GrassApp(HINSTANCE hInstance);
	~GrassApp();

protected:
	virtual void Build() override;

private:
	void BuildGrassBuffer();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildGrassGeometry();
	void BuildRenderItems();
	void BuildFrameResources();
	void BuildPSOs();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		GrassApp app(hInstance);
		if (!app.Initialize())
		{
			return 0;
		}
		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

GrassApp::GrassApp(HINSTANCE hInstance)
	: BaseApp(hInstance)
{

}

GrassApp::~GrassApp()
{
	if (md3dDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

void GrassApp::Build()
{
	BuildGrassBuffer();
	BuildRootSignature();
	// BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildGrassGeometry();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();
}

void GrassApp::BuildGrassBuffer()
{
	UINT byteSize = 32 * 5 * sizeof(Bone);

	vector<Bone> bones(32 * 5);
	for (UINT i = 0; i < 5; ++i)
	{
		for (UINT j = 0; j < 32; ++j)
		{
			UINT index = i * 32 + j;

			bones[index].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
			bones[index].Rotation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			bones[index].Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
			bones[index].Mass = 1.0f;
			bones[index].Length = 1.0f;
			UINT parentIndex = MathHelper::Max<int>((i - 1) * 32 + j, -1);
			bones[index].ParentIndex = parentIndex;
		}
	}

	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto uavDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// auto uavDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&uavDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		// D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(mGrassBuffer.GetAddressOf())));

	CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mGrassUploadBuffer.GetAddressOf())));

	void* data = nullptr;
	ThrowIfFailed(mGrassUploadBuffer->Map(0, nullptr, &data));
	memcpy(data, bones.data(), byteSize);
	mGrassUploadBuffer->Unmap(0, nullptr);

	auto toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(mGrassBuffer.Get(),
		// D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
	mCommandList->ResourceBarrier(1, &toCopyDest);

	mCommandList->CopyBufferRegion(mGrassBuffer.Get(), 0, mGrassUploadBuffer.Get(), 0, byteSize);

	auto toUA = CD3DX12_RESOURCE_BARRIER::Transition(mGrassBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	mCommandList->ResourceBarrier(1, &toUA);
}

void GrassApp::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);
	slotRootParameter[3].InitAsUnorderedAccessView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void GrassApp::BuildDescriptorHeaps()
{

}

void GrassApp::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = D3DUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = D3DUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["grassVS"] = D3DUtil::CompileShader(L"Shaders\\Grass.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["grassCS"] = D3DUtil::CompileShader(L"Shaders\\Grass.hlsl", nullptr, "CS", "cs_5_1");
	mShaders["grassGS"] = D3DUtil::CompileShader(L"Shaders\\Grass.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["grassPS"] = D3DUtil::CompileShader(L"Shaders\\Grass.hlsl", nullptr, "PS", "ps_5_1");

	mStdInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	mGrassInputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void GrassApp::BuildGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

	vector<Vertex> vertices(grid.Vertices.size());

	for (int i = 0; i < grid.Vertices.size(); ++i)
	{
		vertices[i].Pos = grid.Vertices[i].Position;
		vertices[i].Normal = grid.Vertices[i].Normal;
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	vector<uint16_t> indices = grid.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	auto geo = make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries[geo->Name] = move(geo);
}

void GrassApp::BuildGrassGeometry()
{
	struct GrassVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int grassCount = 32;

	vector<GrassVertex> vertices(grassCount);
	for (int i = 0; i < grassCount; ++i)
	{
		float x = MathHelper::RandF(-45.0f, 45.0f);
		float y = 0.0f;
		float z = MathHelper::RandF(-45.0f, 45.0f);

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(0.1f, 0.3f);
	}

	vector<uint16_t> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29, 30, 31
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(GrassVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

	auto geo = make_unique<MeshGeometry>();
	geo->Name = "grassGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(GrassVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;
	mGeometries[geo->Name] = move(geo);
}

void GrassApp::BuildRenderItems()
{
	auto grassRitem = make_unique<RenderItem>();
	grassRitem->World = MathHelper::Identity4x4();;
	grassRitem->TexTransform = MathHelper::Identity4x4();
	grassRitem->ObjCBIndex = 1;
	grassRitem->Mat = nullptr;
	grassRitem->Geo = mGeometries["grassGeo"].get();
	grassRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	grassRitem->IndexCount = grassRitem->Geo->DrawArgs["points"].IndexCount;
	grassRitem->StartIndexLocation = grassRitem->Geo->DrawArgs["points"].StartIndexLocation;
	grassRitem->BaseVertexLocation = grassRitem->Geo->DrawArgs["points"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Grass].push_back(grassRitem.get());
	mAllRitems.push_back(move(grassRitem));

	auto landRitem = make_unique<RenderItem>();
	landRitem->World = MathHelper::Identity4x4();
	landRitem->TexTransform = MathHelper::Identity4x4();
	landRitem->ObjCBIndex = 0;
	landRitem->Mat = nullptr;
	landRitem->Geo = mGeometries["landGeo"].get();
	landRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	landRitem->IndexCount = landRitem->Geo->DrawArgs["grid"].IndexCount;
	landRitem->StartIndexLocation = landRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	landRitem->BaseVertexLocation = landRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(landRitem.get());
	mAllRitems.push_back(move(landRitem));
}

void GrassApp::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), 0));
	}
}

void GrassApp::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mStdInputLayout.data(), (UINT)mStdInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC grassPsoDesc = opaquePsoDesc;
	grassPsoDesc.InputLayout = { mGrassInputLayout.data(), (UINT)mGrassInputLayout.size() };
	grassPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["grassVS"]->GetBufferPointer()),
		mShaders["grassVS"]->GetBufferSize()
	};
	grassPsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["grassGS"]->GetBufferPointer()),
		mShaders["grassGS"]->GetBufferSize()
	};
	grassPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["grassPS"]->GetBufferPointer()),
		mShaders["grassPS"]->GetBufferSize()
	};
	grassPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	grassPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&grassPsoDesc, IID_PPV_ARGS(&mPSOs["grass"])));

	D3D12_COMPUTE_PIPELINE_STATE_DESC grassCSPsoDesc = {};
	ZeroMemory(&grassCSPsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

	// grassCSPsoDesc.pRootSignature = mGrassCSRootSignature.Get();
	grassCSPsoDesc.pRootSignature = mRootSignature.Get();
	grassCSPsoDesc.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["grassCS"]->GetBufferPointer()),
		mShaders["grassCS"]->GetBufferSize()
	};
	grassCSPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&grassCSPsoDesc, IID_PPV_ARGS(&mPSOs["grassCS"])));
}