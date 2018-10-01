uniform float beam_len;

void main()
{
	float dist = gl_Vertex.z;
	float s = (dist + 1.0) * 1.6;

	vec4 pos = gl_Vertex * vec4(s, s, 1.0, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * pos;

	float d = dist / beam_len;
	float falloff = min(1.0 - d, 1.0);
	gl_FrontColor = vec4(gl_Color.xyz, gl_Color.a * falloff);
}
