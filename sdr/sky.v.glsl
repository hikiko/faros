varying vec3 local_pos;

void main()
{
	gl_Position = ftransform();
	local_pos = gl_Vertex.xyz;
}
