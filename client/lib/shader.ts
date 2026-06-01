const vertexShaderSource=
`#version 300 es
    layout(location = 0) in vec2 position; // Position x and y
    layout(location = 1) in vec2 texture; // Texture coordinates u and v 
    uniform vec2 resolution;  // global resolution
    out vec2 v_texture; // Output texture coordinates
    void main() {
    vec2 zeroToOne = position / resolution;
    vec2 zeroToTwo = zeroToOne * 2.0; 
    vec2 clipSpace = zeroToTwo - 1.0;
    gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1); // cuz y grows up
    v_texture =texture;
    }
`
const CursorVertexShaderSource = 
`#version 300 es
layout(location = 0) in vec2 position; // Position x and y
uniform vec2 resolution;  // global resolution
void main() {
    vec2 zeroToOne = position / resolution;
    vec2 clipSpace = zeroToOne * 2.0 - 1.0;
    gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1); // cuz y grows up
}
`
const fragmentShaderSource= 
`#version 300 es
precision highp float;
in vec2 v_texture; // Input texture coordinates
uniform sampler2D u_texture; // Input texture png
out vec4 fragColor; // Output color
void main() {
    vec3 textureColor = texture(u_texture, v_texture).rgb;
    float distance = max(min(textureColor.r, textureColor.g),min(max(textureColor.r, textureColor.g), textureColor.b)); 

    float edgeWidth = length(vec2(dFdx(distance), dFdy(distance))) * 0.7071;
    float edgeCenter = 0.55; 
    
    float opacity = smoothstep(edgeCenter - edgeWidth, edgeCenter + edgeWidth, distance);
   
    fragColor = vec4(0.0, 0.0, 256.0, opacity);  // black
}
`
const CursorFragmentShaderSource = 
`#version 300 es
precision highp float;
out vec4 fragColor; // Output color
void main() {
    fragColor = vec4(0.0, 0.0, 0.0, 1.0);  // red
}
`
function compileShader(gl, type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.error("Shader error:", gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
    }
    return shader;
}

function createProgram(gl, vertexShader, fragmentShader) {
    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    return program;
}
export { vertexShaderSource, fragmentShaderSource,CursorFragmentShaderSource, CursorVertexShaderSource,  compileShader, createProgram };