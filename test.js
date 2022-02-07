const axios = require('axios');

const clients = 9;
const eachclient = 10
let s = 0
let e = 0;

function exec() {
    c = Array.from(Array(clients).keys());
    return c.map(createClient);
}

async function createClient({ seq = true }) {
    let ec = eachclient
    if (seq) {
        while (ec) {
            console.log("seq!")
            await wrapped_req()
            ec--;
        }
    }
    else {
        ec = Array.from(Array(eachclient).keys());
        return Promse.all(ec.map(wrapped_req))
    }
}

function handleSucc(res) {
    // console.log("Success", res.toJSON ? res.toJSON() : res)
    s++;
}

function handleErr(res) {
    console.log("Error", res.toJSON ? res.toJSON() : res)
    e++;
}

function wrapped_req() {
    return axios.get('http://localhost:8080/home.html')
        .then(handleSucc)
        .catch(handleErr)
}


Promise.all(exec()).finally(() => {
    console.log("===================");
    console.log(`S ${s}. E ${e}`);
})

