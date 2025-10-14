#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#include "Common.hlsl"
#include "QuaternionUtil.hlsl"
#include "MathUtil.hlsl"

cbuffer cbWind : register(b2)
{
    float3 gWindVelocity;
    float pad0;
};

struct Bone
{
    float3 position;
    float mass;
    float4 rotation;
    float3 velocity;
    float length;
    int parentIndex;
    float3 localPosition;
    float4 localRotation;
};

struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;

    return vout;
}


RWStructuredBuffer<Bone> bones : register(u0);

const float K_RESTORE = 500.0f;
const float K_DAMPING = 10.0f;

void ApplyParent(int boneId)
{
    Bone bone = bones[boneId];
    Bone parentBone = bones[bone.parentIndex];
    
    float3 upVec = float3(0.0f, 1.0f, 0.0f);
    bone.rotation = normalize(QuaternionMultiply(parentBone.rotation, bone.localRotation));
    
    float3 parentUpVec = normalize(QuaternionRotateOptimized(parentBone.rotation, upVec));
    bone.position = parentBone.position + parentUpVec * parentBone.length;

    bones[boneId].rotation = bone.rotation;
    bones[boneId].position = bone.position;
}

void ProcessBone(int boneId)
{
    Bone bone = bones[boneId];
    float3 upVec = float3(0.0f, 1.0f, 0.0f);
    
    float3 oUpVec = normalize(QuaternionRotateOptimized(bone.rotation, upVec));
    float3 tailPos = oUpVec * bone.length;

    float3 torque = cross(tailPos, gWindVelocity);

    float4 restoreError = QuaternionInverse(bone.localRotation);
    float3 restoreTorque = (restoreError.xyz * K_RESTORE) - (bone.velocity * K_DAMPING);

    torque += restoreTorque;

    float I = (1.0f / 3.0f) * bone.mass * bone.length * bone.length;

    // torque = I * a
    float3 a = torque / I;

    bone.velocity += a * gDeltaTime;
    
    float speed = length(bone.velocity);
    
    if (speed > 0.001)
    {
        float3 axis = bone.velocity / speed;
        float angle = speed * gDeltaTime;
        
        float4 deltaRotation = AxisAngleToQuaternion(axis, angle);
        bone.localRotation = normalize(QuaternionMultiply(deltaRotation, bone.localRotation));
    }

    bone.velocity *= 0.9f;

    bones[boneId].localRotation = bone.localRotation;
    bones[boneId].velocity = bone.velocity;
}

[numthreads(32, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
    int boneId = dtid.x;

    ProcessBone(boneId);
    Bone bone = bones[boneId];
    bones[boneId].rotation = bone.localRotation;

        [unroll]
    for (uint j = 1; j < 5; ++j)
    {
        int childBoneId = 32 * j + boneId;
        ApplyParent(childBoneId);
//             ProcessBone(childBoneId);
//             bone = bones[childBoneId];
//             bones[childBoneId].rotation = QuaternionMultiply(bone.rotation, bone.localRotation);
    }
}

[maxvertexcount(12)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f;
    look = normalize(look);
    float3 right = normalize(cross(up, look));
    
    float halfWidth = gin[0].SizeW.x * 0.5f;
    // float halfHeight = gin[0].SizeW.y * 0.5f;

    float4 v0, v1;
    v0 = float4(gin[0].CenterW + right * halfWidth, 1.0f);
    v1 = float4(gin[0].CenterW - right * halfWidth, 1.0f);

    float2 texC0, texC1;
    texC0 = float2(0.0f, 1.0f);
    texC1 = float2(1.0f, 1.0f);

    Bone bone0 = bones[0];

    GeoOut gout0 = (GeoOut) 0.0f;
    gout0.PosH = mul(v0, gViewProj);
    gout0.PosW = v0.xyz;
    gout0.NormalW = look;
    gout0.TexC = texC0;
    gout0.PrimID = primID;

    GeoOut gout1 = (GeoOut) 0.0f;
    gout1.PosH = mul(v1, gViewProj);
    gout1.PosW = v1.xyz;
    gout1.NormalW = look;
    gout1.TexC = texC1;
    gout1.PrimID = primID;
    
    triStream.Append(gout0);
    triStream.Append(gout1);
    
    [unroll]
    for (int i = 0; i < 5; ++i)
    {
        int boneIndex = i * 32 + primID;
        Bone bone = bones[boneIndex];

        float3 upO = QuaternionRotateOptimized(bone.rotation, float3(0.0f, 1.0f, 0.0f));
        right = normalize(cross(upO, look));
        float4 vRight, vLeft;
        float2 texCRight, texCLeft;

        float3 center = gin[0].CenterW + bone.position;
        float3 rightPos = center + right * halfWidth + upO * bone.length;
        float3 leftPos = center - right * halfWidth + upO * bone.length;
        vRight = float4(rightPos, 1.0f);
        vLeft = float4(leftPos, 1.0f);

        texCRight = float2(0.0f, 1.0f - (1 + i) * 0.2f);
        texCLeft = float2(1.0f, 1.0f - (1 + i) * 2.0f);

        GeoOut goutRight = (GeoOut) 0.0f;
        goutRight.PosH = mul(vRight, gViewProj);
        goutRight.PosW = vRight.xyz;
        goutRight.NormalW = look;
        goutRight.TexC = texCRight;
        goutRight.PrimID = primID;
        triStream.Append(goutRight);

        GeoOut goutLeft = (GeoOut) 0.0f;
        goutLeft.PosH = mul(vLeft, gViewProj);
        goutLeft.PosW = vLeft.xyz;
        goutLeft.NormalW = look;
        goutLeft.TexC = texCLeft;
        goutLeft.PrimID = primID;
        triStream.Append(goutLeft);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}