import requests
from bs4 import BeautifulSoup
import json

# URL of the webpage to scrape
url = "https://master.iw4.zip/servers"

# Headers to simulate a real browser request
headers = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36'
}

# Send a GET request to the URL
response = requests.get(url, headers=headers)

# Check if the request was successful
if response.status_code == 200:
    page_content = response.content
    soup = BeautifulSoup(page_content, 'html.parser')

    # Find the target div with id 'H2M_servers'
    target_div = soup.find('div', id='H2M_servers')

    # Prepare a dictionary to hold all scraped data
    scraped_data = {
        'player_count': None,
        'server_count': None,
        'servers': []
    }

    if target_div:
        # Extract player count
        playercount_span = target_div.find('span', class_="badge badge-primary")
        if playercount_span:
            scraped_data['player_count'] = playercount_span.get_text(strip=True)
        else:
            print("Player count not found.")

        # Extract server count
        servercount_span = target_div.find('span', class_="badge badge-secondary")
        if servercount_span:
            scraped_data['server_count'] = servercount_span.get_text(strip=True)
        else:
            print("Server count not found.")

        # Find all <tr> elements with class 'server-row'
        server_rows = target_div.find_all('tr', class_='server-row')

        # Loop through each server row and extract the IP, Port, and other table data
        for row in server_rows:
            # Extract the IP and Port from the attributes
            ip = row.get('data-ip')
            port = row.get('data-port')

            # Extract all <td> elements from the row
            cells = row.find_all('td')

            # Debug: Print the cell contents to understand their structure
            print(f"Row data: {[cell.get_text(strip=True) for cell in cells]}")

            if len(cells) < 5:
                print("Unexpected number of cells in server row.")
                continue

            try:
                # Extract text from each cell
                name = cells[0].get_text(strip=True)
                map_name = cells[1].get_text(strip=True)  # Assuming map name is in the second cell

                # Extract currentPlayers and maxPlayers from 'currentPlayers/maxPlayers' format
                player_count_str = cells[2].get_text(strip=True)
                currentPlayers, maxPlayers = map(int, player_count_str.split('/'))

                # Check if the ping value is numeric
                ping_str = cells[3].get_text(strip=True)
                ping = int(ping_str) if ping_str.isdigit() else None

                # Append the data to the list as a dictionary
                scraped_data['servers'].append({
                    'ip': ip,
                    'port': port,
                    'name': name,
                    'map': map_name,
                    'currentPlayers': currentPlayers,
                    'maxPlayers': maxPlayers,
                    'ping': ping
                })
            except ValueError as e:
                print(f"Error converting data: {e}")
                print(f"Row data: {[cell.get_text(strip=True) for cell in cells]}")

    else:
        print("Target div with id 'H2M_servers' not found.")

    # Save the scraped data to a JSON file
    try:
        with open('server_data.json', 'w') as json_file:
            json.dump(scraped_data, json_file, indent=4)
        print("Data has been written to server_data.json")
    except IOError as e:
        print(f"Failed to write JSON file: {e}")

else:
    print(f"Failed to retrieve the webpage. Status code: {response.status_code}")
