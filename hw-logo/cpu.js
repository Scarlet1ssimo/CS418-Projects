function drawCPU(milliseconds) {
    if (lastSec == 0) lastSec = milliseconds
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    gl.bindVertexArray(geom.vao)
    if (dx > 0.5 || dx < -0.5)
        vx = -vx
    if (dy > 0.5 || dy < -0.5)
        vy = -vy
    if (scale > 0.02 || scale < 0.01)
        vscale = -vscale
    dx = dx + vx
    dy = dy + vy
    scale = scale + vscale
    Object.entries(window.data.attributes).forEach(([name, data]) => {
        gl.bindBuffer(gl.ARRAY_BUFFER, buf)

        // const matList = [m4trans(-4.5, -7, 0), m4scale(0.02, 0.02, 0.02), m4rotZ(milliseconds / 1000), m4trans(-0.4 * Math.cos(milliseconds / 500), -0.8 * Math.sin(milliseconds / 400), 0)];
        // console.log(data)
        const newData = []
        data.forEach((item, index) => {
            newData[index] = [item[0] * scale + dx, item[1] * scale + dy]
        })
        let f32 = new Float32Array(newData.flat())

        gl.bufferData(gl.ARRAY_BUFFER, f32, gl.DYNAMIC_DRAW)
    })

    gl.drawElements(geom.mode, geom.count, geom.type, 0)
    window.pending = requestAnimationFrame(drawCPU)
}

function setupCPUGeomery(geom) {
    var triangleArray = gl.createVertexArray()
    gl.bindVertexArray(triangleArray)

    Object.entries(geom.attributes).forEach(([name, data]) => {
        window.buf = gl.createBuffer()
        gl.bindBuffer(gl.ARRAY_BUFFER, buf)
        let f32 = new Float32Array(data.flat())
        gl.bufferData(gl.ARRAY_BUFFER, f32, gl.DYNAMIC_DRAW)

        let loc = gl.getAttribLocation(program, name)
        gl.vertexAttribPointer(loc, data[0].length, gl.FLOAT, false, 0, 0)
        gl.enableVertexAttribArray(loc)
    })
    window.dx = 0
    window.vx = .006
    window.dy = 0
    window.vy = .004
    window.scale = 0.02
    window.vscale = 0.0001

    var indices = new Uint16Array(geom.triangles.flat())
    var indexBuffer = gl.createBuffer()
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer)
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.DYNAMIC_DRAW)

    return {
        mode: gl.TRIANGLES,
        count: indices.length,
        type: gl.UNSIGNED_SHORT,
        vao: triangleArray
    }
}

/* async functions return a Promise instead of their actual result.
 * Because of that, they can wait for other Promises to be fulfilled,
 * which makes functions that call fetch or other async functions cleaner.
 */

async function CPUChosen() {
    let vs = await fetch('cpu-vertex.glsl').then(res => res.text())
    let fs = await fetch('cpu-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)
    window.data = await fetch('logo.json').then(r => r.json())

    window.geom = setupCPUGeomery(data)
    // drawCPU(gl, programInfo, buffers)
    window.pending = requestAnimationFrame(drawCPU)
}
