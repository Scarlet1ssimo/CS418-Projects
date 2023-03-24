function drawlogo(milliseconds) {
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    const matList = [m4trans(-4.5, -7, 0), m4scale(0.02, 0.02, 0.02), m4rotZ(milliseconds / 1000), m4trans(-0.4 * Math.cos(milliseconds / 500), -0.8 * Math.sin(milliseconds / 400), 0)];
    var modelViewMatrix = IdentityMatrix
    for (m of matList) {
        modelViewMatrix = m4mul(m, modelViewMatrix)
    }
    gl.uniformMatrix4fv(gl.getUniformLocation(program, 'model'), false, modelViewMatrix)

    gl.bindVertexArray(geom.vao)
    gl.drawElements(geom.mode, geom.count, geom.type, 0)
    window.pending = requestAnimationFrame(drawlogo)
}

async function LOGOChosen() {
    let vs = await fetch('logo-vertex.glsl').then(res => res.text())
    let fs = await fetch('logo-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)
    let data = await fetch('logo.json').then(r => r.json())

    window.geom = setupGeomery(data)
    window.pending = requestAnimationFrame(drawlogo)
}
