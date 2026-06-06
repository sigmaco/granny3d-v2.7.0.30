vs.1.1

// dcl_position  v0
// dcl_normal    v1
// dcl_texcoord0 v2

// This should actually contain the composite WORLD+VIEW+PROJ matrix

dp4 r0.x, v0, c8
dp4 r0.y, v0, c9
dp4 r0.z, v0, c10
mov r0.w, c0.w
dp4 oPos.x, r0, c4
dp4 oPos.y, r0, c5
dp4 oPos.z, r0, c6
dp4 oPos.w, r0, c7

//dp4 oPos.x, v0, c4
//dp4 oPos.y, v0, c5
//dp4 oPos.z, v0, c6
//dp4 oPos.w, v0, c7

// Transform the normal by the blended matrix
// (technically we should use the inverse transpose, so this is incorrect for shears or non-uniform scales).
dp3 r5.x, v1, c8
dp3 r5.y, v1, c9
dp3 r5.z, v1, c10

// Can't use nrm in VS1.1, so we do it manually.
dp3 r5.w, r5, r5
rsq r5.w, r5.w
mul r5.xyz, r5, r5.w

// Do some simple lighting.
// clamp(N.L)
dp3 r5.w, r5, c2
max r5.w, r5.w, c0.z

// Add in the light's ambient.
add r5.w, r5.w, c3.w

// And finally multiply by the actual colour of the light.
mul oD0.xyz, r5.w, c3
mov oD0.w, c2.w

// Copy over the texture coords
mov oT0.xy, v2.xy
