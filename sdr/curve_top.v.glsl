#define PI 3.141592653589793
void main()
{
	float s = 0.55 * cos(gl_Vertex.z * PI / 2.0 + PI / 2.0) + 1.02;
	vec4 pos = gl_Vertex * vec4(s, s, 1.0, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_FrontColor = gl_Color;
}
