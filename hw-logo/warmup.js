function drawWarmup(milliseconds) {
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    const connection = gl.POINTS
    const offset = 0                          // unused here, but required
    const count = 6 + (0 | milliseconds / 10) % 1000  // number of vertices to draw
    let countBindPoint = gl.getUniformLocation(program, 'count')
    gl.uniform1i(countBindPoint, count)
    gl.drawArrays(connection, offset, count)
    window.pending = requestAnimationFrame(drawWarmup)
}

/* async functions return a Promise instead of their actual result.
 * Because of that, they can wait for other Promises to be fulfilled,
 * which makes functions that call fetch or other async functions cleaner.
 */

async function WarmupChosen() {
    let vs = await fetch('warmup-vertex.glsl').then(res => res.text())
    let fs = await fetch('warmup-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)

    window.pending = requestAnimationFrame(drawWarmup)
}