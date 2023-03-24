function drawGPU(milliseconds) {
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    gl.bindVertexArray(geom.vao)

    gl.drawElements(geom.mode, geom.count, geom.type, 0)
    window.pending = requestAnimationFrame(drawGPU)
}

/* async functions return a Promise instead of their actual result.
 * Because of that, they can wait for other Promises to be fulfilled,
 * which makes functions that call fetch or other async functions cleaner.
 */

async function GPUChosen() {
    let vs = await fetch('gpu-vertex.glsl').then(res => res.text())
    let fs = await fetch('gpu-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)
    data = await fetch('logo.json').then(r => r.json())

    window.geom = setupGeomery(data)
    // drawCPU(gl, programInfo, buffers)
    window.pending = requestAnimationFrame(drawGPU)
}
