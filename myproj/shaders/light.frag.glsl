#version 330 core

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

in vec3 mynormal;
in vec4 myvertex;
in vec2 mytexture;

out vec4 final_color;

uniform vec4 ka;
uniform vec4 kd;
uniform vec4 ks;
uniform float sh;

uniform sampler2D tex;

vec3 light_position = vec3(1, 1, 1);
vec4 light_color = vec4(1,1,1,1);

uniform vec4 light_positions[16];
uniform vec4 light_colors[16];
uniform vec3 light_directions[16];
uniform int light_types[16];
uniform int numberofLights_shader = 0;
uniform int to_draw = 0;
uniform int draw_texture = 0;
uniform float opacity = 1.0f;


void main (void)
{   
	if (to_draw == 2)
	{
		final_color = kd;
		return;
	}
	
	vec4 texture_color = vec4(1,1,1,1);
	if (draw_texture == 2) {
		texture_color = texture(tex, mytexture.st);
	}

	vec3 eyepos = vec3(0,0,0);

    vec4 _pos = myview_matrix * myvertex;
	vec3 pos = _pos.xyz / _pos.w;

	vec3 normal = normalize(mynormal_matrix * mynormal);
	
	final_color = ka * texture_color;

	for (int i = 0; i < numberofLights_shader; i++)
	{
		if (light_types[i] == 0) { // point-light
			vec4 _lightpos = myview_matrix * light_positions[i];
			vec3 lightpos = _lightpos.xyz / _lightpos.w;
			//vec3 lightpos = light_positions[i];

			vec3 light_to_pos = normalize(pos - lightpos);

			vec3 r = normalize(reflect(light_to_pos, normal));

			vec3 pos_to_eyepos = normalize( eyepos - pos );

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

			final_color += light_colors[i] * kd * cos_theta * texture_color;
			final_color += light_colors[i] * ks * 
						   pow(max( dot(r, pos_to_eyepos), 0.0), sh);
		} else if (light_types[i] == 1) { // directional light
			vec3 light_to_pos = normalize(mynormal_matrix * light_directions[i]);

			vec3 r = normalize(reflect(light_to_pos, normal));

			vec3 pos_to_eyepos = normalize( eyepos - pos );

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

			final_color += light_colors[i] * kd * cos_theta * texture_color;
			final_color += light_colors[i] * ks * 
						   pow(max( dot(r, pos_to_eyepos), 0.0), sh);
		} else if (light_types[i] == 2) { // spot light
			vec4 _lightpos = myview_matrix * light_positions[i];
			vec3 lightpos = _lightpos.xyz / _lightpos.w;
			//vec3 lightpos = light_positions[i];

			vec3 light_to_pos = normalize(pos - lightpos);

			vec3 pos_to_eyepos = normalize( eyepos - pos );

			vec3 r = normalize(reflect(light_to_pos, normal));

			float cos_theta = max( dot(normal, -light_to_pos), 0.0 );

			float theta     = dot(light_to_pos, normalize(mynormal_matrix * light_directions[i]));
			float epsilon   = 0.0448f;
			float intensity = clamp((theta - 0.953f) / epsilon, 0.0, 1.0);  
			
			final_color += light_colors[i] * kd * cos_theta * intensity * texture_color;
			final_color += light_colors[i] * ks * pow(max( dot(r, pos_to_eyepos), 0.0), sh) * intensity;
		}
	}
	final_color.a = opacity;
}
