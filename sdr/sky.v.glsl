void main()
{
	gl_Position = ftransform();

	vec3 p = normalize(gl_Vertex.xyz);
	vec2 uv = vec2(atan(p.z, p.x), asin(p.y));

	gl_TexCoord[0] = vec4(uv, 0.0, 0.0);
}
