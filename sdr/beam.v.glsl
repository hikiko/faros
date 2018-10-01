void main()
{
	float s = gl_Vertex.z + 1.0;

	vec4 pos = gl_Vertex * vec4(s, s, 1.0, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_FrontColor = gl_Color;
}
