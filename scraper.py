import requests
from bs4 import BeautifulSoup
import json

def scrape_data():
    url = "https://master.iw4.zip/servers"
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36'
    }
    
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        page_content = response.content
        soup = BeautifulSoup(page_content, 'html.parser')
        
        target_div = soup.find('div', id='H2M_servers')
        
        scraped_data = {
            'player_count': None,
            'server_count': None,
            'servers': []
        }
        
        if target_div:
            playercount_span = target_div.find('span', class_="badge badge-primary")
            if playercount_span:
                scraped_data['player_count'] = playercount_span.get_text(strip=True)
            
            servercount_span = target_div.find('span', class_="badge badge-secondary")
            if servercount_span:
                scraped_data['server_count'] = servercount_span.get_text(strip=True)
            
            server_rows = target_div.find_all('tr', class_='server-row')
            for row in server_rows:
                ip = row.get('data-ip')
                port = row.get('data-port')
                cells = row.find_all(['td', 'th'])
                cell_texts = [cell.get_text(strip=True) for cell in cells]
                
                try:
                    current_players, max_players = map(int, cell_texts[2].split('/'))
                except ValueError:
                    current_players = 0
                    max_players = 0
                
                scraped_data['servers'].append({
                    'ip': ip,
                    'port': port,
                    'name': cell_texts[0],
                    'map': cell_texts[1],
                    'currentPlayers': current_players,
                    'maxPlayers': max_players
                })
        
        with open('server_data.json', 'w') as json_file:
            json.dump(scraped_data, json_file, indent=4)
        print("Data has been written to server_data.json")
    else:
        print(f"Failed to retrieve the webpage. Status code: {response.status_code}")

if __name__ == "__main__":
    scrape_data()
