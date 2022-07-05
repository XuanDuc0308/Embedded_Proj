
<?php



    $host = "localhost";    // host = localhost because database hosted on the same server where PHP files are hosted
    $dbname = "tsts_db";    // Database name
    $username = "root";		// Database username
    $password = "";	        // Database password


// Establish connection to MySQL database
$conn = new mysqli($host, $username, $password, $dbname);


// Check if connection established successfully
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

else {}


// Select values from MySQL database table

$sql = "SELECT ticket_id FROM tickets";  // Update your tablename here

$result = $conn->query($sql);

if ($result->num_rows > 0) {

    // output data of each row
    while($row = $result->fetch_assoc()) {
        echo $row["ticket_id"];
    
}
} else {
    echo "0 results";
}
;

$conn->close();



?>
