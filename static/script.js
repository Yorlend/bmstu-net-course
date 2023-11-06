window.onload = function() {
    let value = 0
    const counter = document.getElementById('counter')
    setInterval(() => counter.innerText = ++value, 1000)
}
