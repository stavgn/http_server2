const axios = require('axios');

const clients = 4
const eachclient = 1
let s = 0
let e = 0;

function exec() {
    c = Array.from(Array(clients).keys())
    return c.map(createClient);
}

async function createClient({ seq = true }) {
    let ec = eachclient
    if (seq) {
        while (ec) {
            await wrapped_req_dynamic(1)
            ec--;
        }
    }
    else {
        ec = Array.from(Array(eachclient).keys())
        return Promse.all(ec.map(wrapped_req))
    }
}

function handleSucc(res) {
    data = res.toJSON ? res.toJSON() : res;
    console.log(data.headers);
    s++;
}

function handleErr(res) {
    console.log("Error", res.toJSON ? res.toJSON() : res)
    if (res.status == '404') {
        s++;
    }
    else {
        e++;
    }
}

function wrapped_req_home() {
    return axios.get('http://localhost:8080/home.html')
        .then(handleSucc)
        .catch(handleErr)
}

function wrapped_req_favicon() {
    return axios.get('http://localhost:8080/favicon.ico')
        .then(handleSucc)
        .catch(handleErr)
}

function wrapped_req_dynamic(timeout = 0.3) {
    return axios.get(`http://localhost:8080/output.cgi?${timeout}`)
        .then(handleSucc)
        .catch(handleErr)
}


Promise.all(exec()).finally(() => {
    console.log("===================");
    console.log(`S ${s}. E ${e}`);
})

