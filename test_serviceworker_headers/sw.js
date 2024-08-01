self.addEventListener('fetch', function (e) {
    let urlObj = new URL(e.request.url);
    console.log(`SW fetching: ${urlObj}`);
});

self.addEventListener('install', function (event) {
    //console.log("sw.js install event:" + event);
    event.waitUntil(self.skipWaiting()); // Activate worker immediately
});

self.addEventListener('activate', function (event) {
    // console.log("sw.js activate event:" + event);
    event.waitUntil(self.clients.claim()); // Become available to all pages
});