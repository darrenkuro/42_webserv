// Function to get the value of a cookie by name
function getCookie(cookieName) {
	const cookies = document.cookie.split('; ');
	for (const cookie of cookies) {
		const [name, value] = cookie.split('=');
		if (name === cookieName) {
			return decodeURIComponent(value);
		}
	}
	return null;
}

// Function to display the username
function displayUsername() {
	const username = getCookie('username');
	const usernameDisplay = document.getElementById('usernameDisplay');

	if (username) {
		usernameDisplay.innerHTML = `Welcome, ${username}!`;
	} else {
		usernameDisplay.innerHTML = 'Guest';
	}
}

// Call the function when the page loads
window.onload = displayUsername;
