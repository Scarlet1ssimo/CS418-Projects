function addNormals(data) {
    let normals = new Array(data.attributes.position.length)
    for (let i = 0; i < normals.length; i += 1) normals[i] = new Array(3).fill(0)
    for ([i0, i1, i2] of data.triangles) {
        // find the vertex positions
        let p0 = data.attributes.position[i0]
        let p1 = data.attributes.position[i1]
        let p2 = data.attributes.position[i2]
        // find the edge vectors and normal
        let e0 = m4sub_(p0, p2)
        let e1 = m4sub_(p1, p2)
        let n = m4cross_(e0, e1)
        // loop over x, y and z
        for (let j = 0; j < 3; j += 1) {
            // add a coordinate of a normal to each of the three normals
            normals[i0][j] += n[j]
            normals[i1][j] += n[j]
            normals[i2][j] += n[j]
        }
    }
    for (let i = 0; i < normals.length; i += 1) normals[i] = m4normalized_(normals[i])
    data.attributes.normal = normals;
}
function timeStep(milliseconds) {
    let seconds = milliseconds / 1000;
    let s2 = Math.cos(seconds / 2) - 1

    let eye = [2 * Math.cos(s2), 2 * Math.sin(s2), 1];
    window.v = m4view(eye, [0, 0, 0], [0, 0, 1])
    gl.uniform3fv(gl.getUniformLocation(program, 'eyedir'), new Float32Array(m4normalized_(eye)))
    if (options.RotatingLight)
        gl.uniform3fv(gl.getUniformLocation(program, 'lightdir'), new Float32Array(normalize([Math.cos(seconds / 2), Math.sin(seconds / 2), 1])))
    else gl.uniform3fv(gl.getUniformLocation(program, 'lightdir'), [0.8, -0.6, 1])

    draw()
    window.pending = requestAnimationFrame(timeStep)
}
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

    let program = gl.createProgram()
    gl.attachShader(program, vs)
    gl.attachShader(program, fs)
    gl.linkProgram(program)
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        console.error(gl.getProgramInfoLog(program))
        throw Error("Linking failed")
    }

    return program
}
function setupGeomery(geom, program) {
    var triangleArray = gl.createVertexArray()
    gl.bindVertexArray(triangleArray)

    for (let name in geom.attributes) {
        let data = geom.attributes[name]
        supplyDataBuffer(data, program, name)
    }

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
function supplyDataBuffer(data, program, vsIn, mode) {
    if (mode === undefined) mode = gl.STATIC_DRAW

    let buf = gl.createBuffer()
    gl.bindBuffer(gl.ARRAY_BUFFER, buf)
    let f32 = new Float32Array(data.flat())
    gl.bufferData(gl.ARRAY_BUFFER, f32, mode)

    let loc = gl.getAttribLocation(program, vsIn)
    gl.vertexAttribPointer(loc, data[0].length, gl.FLOAT, false, 0, 0)
    gl.enableVertexAttribArray(loc)

    return buf;
}


function randomRealInterval(min, max) { // [min,max]
    return Math.random() * (max - min) + min
}

function faultingMethod(gSize, n, sph, hyd) {
    let data = {
        "attributes":
        {
            "position": [],
        },
        "triangles": []
    }
    let z = [];
    for (let i = 0; i < gSize; i++) {
        z[i] = [];
        for (let j = 0; j < gSize; j++) {
            z[i][j] = 0
        }
    }
    let delta = 1
    for (let i = 0; i < n; i++) {
        let p = [randomRealInterval(0, gSize - 1), randomRealInterval(0, gSize - 1)]
        theta = randomRealInterval(0, 2 * Math.PI)
        let n = [Math.cos(theta), Math.sin(theta)]
        for (let i = 0; i < gSize; i++) {
            for (let j = 0; j < gSize; j++) {
                let b = [i, j]
                let r = dot(n, sub(b, p))
                let R = gSize
                let g = (r) => r < R ? (1 - (r / R) ** 2) ** 2 : 0
                if (r >= 0)
                    z[i][j] += delta * g(Math.abs(r))
                else
                    z[i][j] -= delta * g(Math.abs(r))
            }
        }
        delta *= randomRealInterval(options.decay, 1)
    }
    for (let i = 0; i < sph; i++) {
        let z_ = z
        for (let i = 1; i < gSize - 1; i++) {
            for (let j = 1; j < gSize - 1; j++) {
                z[i][j] = (z[i][j] + z_[i + 1][j] + z_[i][j + 1] + z_[i - 1][j] + z_[i][j - 1]) / 5
                // This is equivalent to moving towards the average 
                // z[i][j] = (z_[i + 1][j] + z_[i][j + 1] + z_[i - 1][j] + z_[i][j - 1]) / 4
            }
        }
    }
    for (let i = 0; i < hyd; i++) {
        // ?
    }
    var min = Math.min(...z.flat()),
        max = Math.max(...z.flat()),
        c = randomRealInterval(1 / 4, 1 / 2)
    options.maxmin = [c / 2, - c / 2]

    for (let i = 0; i < gSize; i++)
        for (let j = 0; j < gSize; j++)
            data.attributes.position.push([i / gSize - .5, j / gSize - .5, ((z[i][j] - min) / (max - min) - .5) * c])
    for (let i = 1; i < gSize; i++)
        for (let j = 1; j < gSize; j++) {
            const get = (i, j) => i * gSize + j
            let a = get(i - 1, j - 1),
                b = get(i - 1, j),
                c = get(i, j - 1),
                d = get(i, j)
            data.triangles.push([b, a, c])
            data.triangles.push([d, b, c])
        }
    return data
}

function icosphere(n) {
    let phi = 5 ** 0.5 / 2 - 1 / 2
    let c = 1 / 2;//scale
    let points = [
        [1, phi, 0], [-1, phi, 0], [1, -phi, 0], [-1, -phi, 0],
        [0, 1, phi], [0, -1, phi], [0, 1, -phi], [0, -1, -phi],
        [phi, 0, 1,], [phi, 0, -1,], [-phi, 0, 1,], [-phi, 0, -1,],
    ]


    let triangles = []
    for (let i = 0; i < 12; i++)
        for (let j = i + 1; j < 12; j++)
            for (let k = j + 1; k < 12; k++) {
                if (mag(sub(points[i], points[j])) <= 2 * phi + 0.001 &&
                    mag(sub(points[k], points[j])) <= 2 * phi + 0.001 &&
                    mag(sub(points[i], points[k])) <= 2 * phi + 0.001) {
                    // triangles.push([j, i, k])
                    if (dot(cross(points[i], points[j]), points[k]) > 0)
                        triangles.push([i, j, k])
                    else
                        triangles.push([i, k, j])
                }
            }
    for (let i = 0; i < n; i++) {
        let triangles_ = []
        let N = points.length
        let S = {}
        let getMidpointIdx = (i, j) => { //simulate a (i,j) -> x map
            if (i > j) [i, j] = [j, i]
            pair = i * N + j
            if (pair in S) return S[pair]
            let l = points.length
            let a = points[i], b = points[j]
            let m = mag(a);
            let interp = mul(normalize(add(a, b)), m)
            points[l] = interp
            return S[pair] = l
        }
        triangles.forEach((v) => {
            //      i
            //
            //   km    jm
            //
            //j     im     k
            let [i, j, k] = v
            let im = getMidpointIdx(j, k)
            let jm = getMidpointIdx(i, k)
            let km = getMidpointIdx(i, j)
            triangles_.push([i, km, jm])
            triangles_.push([j, im, km])
            triangles_.push([k, jm, im])
            triangles_.push([im, jm, km])
        })
        triangles = triangles_
    }
    points.forEach((v, i, a) => points[i] = mul(v, c))
    options.maxmin = [c, -c]
    return {
        "attributes":
        {
            "position": points,
        },
        "triangles": triangles
    }
}

function torus(nr, np, r1, r2) {
    let points = []
    let triangles = []
    let getindex = (r, p) => (r % nr) + (p % np) * nr
    for (j = 0; j < np; j++) {
        let t2 = 2 * Math.PI * j / np
        let r_ = r1 - Math.cos(t2) * r2
        let z_ = Math.sin(t2) * r2
        for (let i = 0; i < nr; i++) {
            let t1 = 2 * Math.PI * i / nr
            let ci = [r_ * Math.cos(t1), r_ * Math.sin(t1), z_]
            points.push(ci)
            triangles.push([getindex(i + 1, j), getindex(i, j), getindex(i, j + 1)])
            triangles.push([getindex(i + 1, j + 1), getindex(i + 1, j), getindex(i, j + 1)])
        }
    }
    options.maxmin = [r2, -r2]
    return {
        "attributes":
        {
            "position": points,
        },
        "triangles": triangles
    }
}

function UVsphere(lat, long) {
    let r = .5
    let points = [[0, 0, r], [0, 0, -r]]
    let triangles = []
    let getindex = (lat_idx, long_idx) => {
        if (lat_idx == 0) return 0
        if (lat_idx == lat - 1) return 1
        return (lat_idx - 1) * long + (long_idx % long) + 2
    }
    for (let i = 1; i < lat - 1; i++) {
        let t1 = Math.PI * i / (lat - 1)
        let r_ = Math.sin(t1) * r
        let z_ = Math.cos(t1) * r
        for (let j = 0; j < long; j++) {
            let t2 = 2 * Math.PI * j / long
            points.push([Math.cos(t2) * r_, Math.sin(t2) * r_, z_])
        }
    }
    for (let i = 2; i < lat - 1; i++) {
        for (let j = 0; j < long; j++) {
            triangles.push([getindex(i - 1, j), getindex(i, j), getindex(i - 1, j + 1)])
            triangles.push([getindex(i - 1, j + 1), getindex(i, j), getindex(i, j + 1)])
        }
    }
    for (let j = 0; j < long; j++) {
        triangles.push([getindex(0, j), getindex(1, j), getindex(1, j + 1)])
        triangles.push([getindex(lat - 2, j), getindex(lat - 1, j), getindex(lat - 2, j + 1)])
    }

    options.maxmin = [r, -r]
    return {
        "attributes":
        {
            "position": points,
        },
        "triangles": triangles
    }
}