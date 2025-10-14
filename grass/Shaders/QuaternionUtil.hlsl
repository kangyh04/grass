float3 QuaternionRotateOptimized(float4 q, float3 v)
{
    float3 qvec = q.xyz;
    float3 uv = cross(qvec, v);
    float3 uuv = cross(qvec, uv);
    return v + ((uv * q.w) + uuv) * 2.0f;
}

float4 QuaternionMultiply(float4 a, float4 b)
{
    return float4(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

float4 AxisAngleToQuaternion(float3 axis, float angle)
{
    float halfAngle = angle * 0.5f;
    return float4(axis * sin(halfAngle), cos(halfAngle));
}

float4 QuaternionConjugate(float4 quaternion)
{
    return float4(-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w);
}

float4 QuaternionInverse(float4 quaternion)
{
    float magnitudeSq = dot(quaternion, quaternion);
    
    return QuaternionConjugate(quaternion) / magnitudeSq;
}