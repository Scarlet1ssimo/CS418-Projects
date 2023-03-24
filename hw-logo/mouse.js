function getMousePos(canvas, evt) {
    var rect = canvas.getBoundingClientRect();
    return {
        x: (evt.clientX - rect.left) / 250 - 1,
        y: -((evt.clientY - rect.top) / 250 - 1)
    };
}
function drawMouse(milliseconds) {
    gl.clear(gl.COLOR_BUFFER_BIT)
    gl.useProgram(program)
    let secondsBindPoint = gl.getUniformLocation(program, 'seconds')
    gl.uniform1f(secondsBindPoint, milliseconds / 1000)
    gl.bindVertexArray(geom.vao)
    td = (milliseconds - lastSec) / 10000
    B = .8
    ax = ay = 0
    if (typeof mousePos !== 'undefined') {
        eps = 5e-2
        ddx = mousePos.x - dx
        ddy = mousePos.y - dy
        if (Math.abs(ddx) < eps)
            ddx = ddx > 0 ? eps : -eps
        if (Math.abs(ddy) < eps)
            ddy = ddy > 0 ? eps : -eps
        GMm = 100.0
        ax = GMm * ddx / Math.pow((ddx * ddx + ddy * ddy), 3 / 2)
        ay = GMm * ddy / Math.pow((ddx * ddx + ddy * ddy), 3 / 2)
    }
    vx += ax * td
    vy += ay * td
    dx = dx + vx * td
    dy = dy + vy * td
    if (dx < -B || dx > B) {
        dx = Math.min(Math.max(dx, -B), B)
        vx = 0
    }
    if (dy < -B || dy > B) {
        dy = Math.min(Math.max(dy, -B), B)
        vy = 0
    }
    Object.entries(window.data.attributes).forEach(([name, data]) => {
        gl.bindBuffer(gl.ARRAY_BUFFER, buf)
        const newData = []
        data.forEach((item, index) => {
            newData[index] = [(item[0] - 4.5) * scale + dx, (item[1] - 7) * scale + dy]
        })
        let f32 = new Float32Array(newData.flat())

        gl.bufferData(gl.ARRAY_BUFFER, f32, gl.DYNAMIC_DRAW)
    })

    gl.drawElements(geom.mode, geom.count, geom.type, 0)
    window.pending = requestAnimationFrame(drawMouse)
    window.lastSec = milliseconds
}

function setupMouseGeomery(geom) {
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
    window.dx = .5
    window.dy = .5
    window.vx = 1
    window.vy = -1
    window.scale = 0.02
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

async function MouseChosen() {
    var canvas = document.getElementById("canvas");
    canvas.addEventListener('mousemove', function (evt) {
        window.mousePos = getMousePos(canvas, evt);
        // console.log('Mouse position: ' + mousePos.x + ',' + mousePos.y);
    }, false);
    let vs = await fetch('mouse-vertex.glsl').then(res => res.text())
    let fs = await fetch('mouse-fragment.glsl').then(res => res.text())
    compileAndLinkGLSL(vs, fs)
    window.data = await fetch('logo.json').then(r => r.json())

    window.geom = setupMouseGeomery(data)
    // drawMouse(gl, programInfo, buffers)
    window.pending = requestAnimationFrame(drawMouse)
}
