function collision(state1, state2) {
    var p1 = []
    var p2 = []
    data.triangles.forEach((tri, index) => {
        o = [state1.dx, state1.dy]
        p1[index] = [add(o, mul(data.attributes.position[tri[0]], scale)),
        add(o, mul(data.attributes.position[tri[1]], scale)),
        add(o, mul(data.attributes.position[tri[2]], scale)),]
    })
    data.triangles.forEach((tri, index) => {
        o = [state2.dx, state2.dy]
        p2[index] = [add(o, mul(data.attributes.position[tri[0]], scale)),
        add(o, mul(data.attributes.position[tri[1]], scale)),
        add(o, mul(data.attributes.position[tri[2]], scale)),]
    })
    coll = false
    p1.forEach(([a, b, c]) => {
        // var sign = cross(sub(a, b), sub(b, c))
        p2.flat().forEach((d) => {
            var s1 = cross(sub(a, b), sub(b, d))
            var s2 = cross(sub(b, c), sub(c, d))
            var s3 = cross(sub(c, a), sub(a, d))
            if (s1 * s2 > 0 && s2 * s3 > 0 && s3 * s1 > 0) {
                coll = true
                return
            }
        })
        if (coll) return
    })
    return coll
}
function process(seconds) {
    B = 0.8
    if (state1.dx > B || state1.dx < -B)
        state1.vx = -state1.vx
    if (state1.dy > B || state1.dy < -B)
        state1.vy = -state1.vy
    state1.dx = state1.dx + state1.vx * seconds
    state1.dy = state1.dy + state1.vy * seconds
    if (state2.dx > B || state2.dx < -B)
        state2.vx = -state2.vx
    if (state2.dy > B || state2.dy < -B)
        state2.vy = -state2.vy
    state2.dx = state2.dx + state2.vx * seconds
    state2.dy = state2.dy + state2.vy * seconds
    if (collision(state1, state2)) {
        if (Math.abs(state1.dy - state2.dy) <= 9 * scale) {
            tmp = state1.vx;
            state1.vx = state2.vx;
            state2.vx = tmp;
        }
        if (Math.abs(state1.dx - state2.dx) <= 14 * scale) {
            tmp = state1.vy;
            state1.vy = state2.vy;
            state2.vy = tmp;
        }
    }
}
function drawOne(state) {
    Object.entries(window.data.attributes).forEach(([name, data]) => {
        gl.bindBuffer(gl.ARRAY_BUFFER, buf)
        const newData = []
        data.forEach((item, index) => {
            newData[index] = [(item[0] - 4.5) * scale + state.dx, (item[1] - 7) * scale + state.dy]
        })
        let f32 = new Float32Array(newData.flat())

        gl.bufferData(gl.ARRAY_BUFFER, f32, gl.DYNAMIC_DRAW)
    })
    gl.drawElements(geom.mode, geom.count, geom.type, 0)
}
function drawColl(milliseconds) {
    if (lastSec == 0) lastSec = milliseconds
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    gl.bindVertexArray(geom.vao)
    process((milliseconds - window.lastSec) / 1000)
    drawOne(state1)
    drawOne(state2)

    window.pending = requestAnimationFrame(drawColl)
    window.lastSec = milliseconds
}

function setupCollGeomery(geom) {
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
    window.state1 = { dx: -.3, dy: -.6, vx: .6, vy: .4 }
    window.state2 = { dx: .6, dy: .3, vx: -.8, vy: -.6 }
    window.scale = 0.05
    window.lastSec = 0

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

async function CollChosen() {
    let vs = await fetch('coll-vertex.glsl').then(res => res.text())
    let fs = await fetch('coll-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)
    window.data = await fetch('logo.json').then(r => r.json())

    window.geom = setupCollGeomery(data)
    // drawColl(gl, programInfo, buffers)
    window.pending = requestAnimationFrame(drawColl)
}
