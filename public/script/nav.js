function home() {
	const body = document.getElementById('insert');
	body.innerHTML = "";
	const listItem = document.createElement('p');
	listItem.textContent = 'This is a sample page for the 42 project Webserv. \
		Blah blah blah ....';
	body.appendChild(listItem);
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
	const cookiesHTML = ``;
	const body = document.getElementById('insert');
	body.innerHTML = cookiesHTML;
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
