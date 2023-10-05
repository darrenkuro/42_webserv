function home() {
	const body = document.getElementById('insert');
	body.innerHTML = `
		<p>This is a sample page for the 42 project webserv developed by Luca and Darren in CPP.</p> \
		<p>For the mandatory part, it serves as a simple webserver like nginx, \
		that handles GET, POST, DELETE requests. \
		It takes a configuration file as the first argument or uses a default path. \
		It can handle multiple servers and routes to the correct one given the correct configuration file.</p> \
		<p>For the bonus part, it is able to handle both python and php-cgi for cgi pathway (cgi-bin).
		It stores Cookies and user sessions also.
		The buttons above showcases the simple functionalities.</p>
	`;
}

function upload() {
	const formHTML = `
	<form action="upload" method="post" enctype="multipart/form-data">
		<label for="file">Choose a file:</label>
		<input type="file" name="file" id="file"><br>
		<input type="submit" value="Upload">
	</form>
	<form id="deleteForm" action="delete" method="delete">
		<label for="inputString">Enter a file to delete:</label>
		<input type="text" name="inputString" id="inputString" required>
		<input type="submit" value="Delete">
	</form>`;
	const body = document.getElementById('insert');
	body.innerHTML = formHTML;
	document.getElementById('deleteForm').addEventListener('submit', function(e) {
        e.preventDefault(); // Prevent the default form submission

        var inputString = document.getElementById('inputString').value;
		var deleteUrl = "delete/" + encodeURIComponent(inputString);

        fetch(deleteUrl, {
            method: 'DELETE',
        })
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.text();
        })
        .then(data => {
            console.log(data);  // Handle successful response data here
        })
        .catch(error => {
            console.log('There was a problem with the fetch operation:', error.message);
        });
    });
}

function cookies() {
	const cookiesHTML = `
	<form onsubmit="setCookieFromForm(); return false;">
		<label for="name">Name:</label>
		<input type="text" id="name" name="name" required>
		<input type="submit" value="Submit">
	</form>`;
	const body = document.getElementById('insert');
	body.innerHTML = cookiesHTML;
}

function setCookieFromForm() {
	let nameValue = document.getElementById('name').value;
	setCookie('username', nameValue, 365);
	alert('Cookie set successfully!');
	displayUsername();
}

function setCookie(name, value, days) {
	let date = new Date();
	date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
	let expires = "expires=" + date.toUTCString();
	document.cookie = name + "=" + value + ";" + expires + ";path=/";
}

function cgi() {
	const cgiHTML =`
	<form action="/cgi-bin/example.py" method="post">
		<label for="stdin">Enter something to pipe into stdin for cgi:</label>
		<input type="text" name="stdin" required>
		<input type="submit" value="Submit">
	</form>`;
	const body = document.getElementById('insert');
	body.innerHTML = cgiHTML;
}
