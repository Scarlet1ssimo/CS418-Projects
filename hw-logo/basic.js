const IdentityMatrix = new Float32Array([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1])
function compileAndLinkGLSL(vs_source, fs_source) {
    let vs = gl.createShader(gl.VERTEX_SHADER)
    gl.shaderSource(vs, vs_source)
    gl.compileShader(vs)
    if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
        console.error(gl.getShaderInfoLog(vs))
        throw Error("Vertex shader compilation failed")
    }

    let fs = gl.createShader(gl.FRAGMENT_SHADER)
    gl.shaderSource(fs, fs_source)
    gl.compileShader(fs)
    if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
        console.error(gl.getShaderInfoLog(fs))
        throw Error("Fragment shader compilation failed")
    }

    window.program = gl.createProgram()
    gl.attachShader(program, vs)
    gl.attachShader(program, fs)
    gl.linkProgram(program)
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        console.error(gl.getProgramInfoLog(program))
        throw Error("Linking failed")
    }
}

function setupGeomery(geom) {
    var triangleArray = gl.createVertexArray()
    gl.bindVertexArray(triangleArray)

    Object.entries(geom.attributes).forEach(([name, data]) => {
        let buf = gl.createBuffer()
        gl.bindBuffer(gl.ARRAY_BUFFER, buf)
        let f32 = new Float32Array(data.flat())
        gl.bufferData(gl.ARRAY_BUFFER, f32, gl.STATIC_DRAW)

        let loc = gl.getAttribLocation(program, name)
        gl.vertexAttribPointer(loc, data[0].length, gl.FLOAT, false, 0, 0)
        gl.enableVertexAttribArray(loc)
    })

    var indices = new Uint16Array(geom.triangles.flat())
    var indexBuffer = gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer)
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW)

    return {
        mode: gl.TRIANGLES,
        count: indices.length,
        type: gl.UNSIGNED_SHORT,
        vao: triangleArray
    }
}

async function radioChanged() {
    let chosen = document.querySelector('input[name="example"]:checked').value
    cancelAnimationFrame(window.pending)
    window[chosen + 'Chosen']()
}

/* async functions return a Promise instead of their actual result.
 * Because of that, they can wait for other Promises to be fulfilled,
 * which makes functions that call fetch or other async functions cleaner.
 */
async function setup(event) {
    window.gl = document.querySelector('canvas').getContext('webgl2')
    document.querySelectorAll('input[name="example"]').forEach(elem => {
        elem.addEventListener('change', radioChanged)
    })
    radioChanged()
}

window.addEventListener('load', setup)