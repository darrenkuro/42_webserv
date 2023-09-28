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
	</form>`;
	const body = document.getElementById('insert');
	body.innerHTML = formHTML;
}
