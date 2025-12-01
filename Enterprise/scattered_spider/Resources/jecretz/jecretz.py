import json
import requests
import sys
import re
import argparse
from truffleHogRegexes.regexChecks import regexes

# object to contain card data and output it neatly
class Card:
	def __init__(self, _id, boardid, listid, description, customfields):
		self.id = _id
		self.boardid = boardid
		self.listid = listid
		self.description = description
		self.customfields = customfields

	def __str__(self):
		return f"id: {self.id}\n\tboardid: {self.boardid}, listid: {self.listid}\n\tdescription: {self.description}\n\tcustomfields: {self.customfields}\n"

def main():
	# parse command line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument("-w", "--wekan-url", help="url of the wekan site to scrape, must be in the format http://[URL]/")
	parser.add_argument("-u", "--username", help="username to authenticate to wekan with")
	parser.add_argument("-p", "--password", help="password to authenticate to wekan with")
	parser.add_argument("-v", "--verbose", action="store_true", help="toggle verbose standard output")
	args = parser.parse_args()

	verbose = False
	wekanurl = ""
	username = ""
	password = ""

	if args.verbose:
		verbose = True
	if args.wekan_url:
		if not args.wekan_url.startswith('http://') or not args.wekan_url.endswith('/'):
			print(f'[ERROR] Incorrect format for wekan url: {args.wekan_url}.\n  Must be in the format http://[URL]/')
			sys.exit()
		wekanurl = args.wekan_url
	if args.username:
		username = args.username
	if args.password:
		password = args.password

	if not wekanurl or not username or not password:
		parser.print_help()
		sys.exit()

	# output parsed arguments
	if verbose:
		print('Running with verbose output\n')
		print('Startup info from arguments:')
		print(f'wekanurl: {wekanurl}')
		print(f'username: {username}')
		print(f'password: {password}')

	# ------- API URL GENERATION START -----------
	# From Wekan API Python CLI
	# strings to build API URLs

	loginurl = 'users/login'
	wekanloginurl = wekanurl + loginurl
	apiboards = 'api/boards/'
	apiuser = 'api/user'
	s = '/'
	l = 'lists'
	sw = 'swimlane'
	sws = 'swimlanes'
	cs = 'cards'
	user = wekanurl + apiuser

	# ------- API URL GENERATION END -----------

	# ------- LOGIN TOKEN START -----------
	# From Wekan API Python CLI

	# get API key from server using provided credentials
	print('\nGetting API key from server')

	data = {"username": username, "password": password}
	try:
		body = requests.post(wekanloginurl, json=data)
	except:
		print(f'[ERROR] Unable to connect to the wekan login endpoint at {wekanloginurl}.')
		sys.exit()
	d = body.json()
	if verbose:
		print(f'api key request response: {d}')

	# if the response does not have a token, getting the API key failed
	try:
		apikey = d['token']

	except:
		print('[ERROR] Unable to retrieve API key:')
		print(f'  Error:  {d['error']}')
		print(f'  Reason: {d['reason']}')
		sys.exit()

	if not apikey:
		print('[ERROR] Unable to retrieve API key:')
		print(f'\n  Error: {d['error']}\n  Reason: {d['reason']}')
		sys.exit()

	if verbose:
		print(f'apikey: {apikey}')

	# ------- LOGIN TOKEN END -----------

	# build request headers
	headers = {'Accept': 'application/json', 'Authorization': 'Bearer {}'.format(apikey)}

	# get board ids we have access to
	print("\nGetting board ids")
	boardids = []
	body = requests.get(user, headers=headers)
	for i in body.json()['boards']:
		boardids.append(i['boardId'])

	if verbose:
		print("\nBoards:")
		print(boardids)

	# get swimlane ids within boards
	print("\nGetting swimlane ids")
	swimlaneids = {}
	for boardid in boardids:
		swimlanes = wekanurl + apiboards + boardid + s + sws
		body = requests.get(swimlanes, headers=headers)
		for i in body.json():
			swimlaneids[i['_id']] = boardid

	if verbose:
		print("\nSwimlanes:")
		print(swimlaneids)

	# get cards in swimlanes
	print("\nGetting cards")
	cards = []
	for swimlaneid, boardid in swimlaneids.items():
		cardsurl = wekanurl + apiboards + boardid + s + sws + s + swimlaneid + s + cs
		body = requests.get(cardsurl, headers=headers)
		for i in body.json():
			cardsurl = wekanurl + apiboards + boardid + s + l + s + i['listId'] + s + cs + s + i['_id']
			card_body = requests.get(cardsurl, headers=headers)
			cf = card_body.json().get('customFields', dict())
			cards.append(Card(i['_id'], boardid, i['listId'], i['description'], cf))

	if verbose:
		print("\nCards:")
		for card in cards:
			print(card)

	print("Searching for secrets in card descriptions and custom fields")

	custom_rules = {
		"json_data": "\\\"?(?:[c|C][l|L][i|I][e|E][n|N][t|T][i|I][d|D]|[p|P][a|A][s|S][s|S][w|W][o|O][r|R][d|D]|[a|A][u|U][t|T][h|H]|[t|T][o|O][k|K][e|E][n|N]|[s|S][e|E][c|C][r|R][e|E][t|T]|[p|P][w|W][d|D]|[p|P][a|A][s|S][s|S]|[p|P][a|A][s|S][s|S][w|W][d|D])\\\"?\\s?:\\s?\\\"?[a-zA-Z0-9\\/@#$!%^&*()\\[\\]{}\\\\+=-]+\\\"?",
		"equals": "(?:[A|a][u|U][t|T][h|H]|[p|P][a|A][s|S][s|S][w|W][o|O][r|R][d|D]|[t|T][o|O][k|K][e|E][n|N]|[s|S][e|E][c|C][r|R][e|E][t|T]|[p|P][w|W][d|D]|[p|P][a|A][s|S][s|S]|[p|P][a|A][s|S][s|S][w|W][d|D])\\s?=\\s?[a-zA-Z0-9\\/@#$!%^&*()\\[\\]{}]+",
		"headers": "(?:[A|a]uthorization|[t|T]oken|[a|A]ccess-[t|T]oken|access_token|[a|A]uth_token|[s|S][e|E][c|C][r|R][e|E][t|T]|[c|C][o|O][o|O][k|K][i|I][e|E]|[p|P][w|W][d|D]|[p|P][a|A][s|S][s|S]|[p|P][a|A][s|S][s|S][w|W][d|D])\\s?:\\s?(?:Bearer\\s)?[a-zA-Z0-9.\\-=\\/+_\\\\!]+",
		"jwt": "eyJ[a-zA-Z0-9.=/+-]+",
		"mongo": '(?:mongodb:\\/\\/)(?:[a-zA-Z0-9\\@_:-]{1,50}\\.)+[a-zA-Z0-9]{1,25}'
	}

	rules = regexes.copy()
	rules.update(custom_rules)
	desc_match = []
	cf_match = []

	# parse each card description and custom field for matches
	for card in cards:
		for rule in rules:
			if card.description:
				match = re.findall(rules[rule], str(card.description))
				if match:
					desc_match.append(match[0])

			if card.customfields:
				for cf in card.customfields:
					if cf.get('value'):
						match = re.findall(rules[rule], str(cf['value']))
						if match:
							cf_match.append(match[0])

	# deduplicate lists
	desc_match = set(desc_match)
	cf_match = set(cf_match)

	# output what we found
	print("\nFound secrets:")
	print("\n[ Descriptions ]")
	for m in desc_match:
		print(f'{m}')
	print("\n[ Custom Fields ]")
	for m in cf_match:
		print(f'{m}')

if __name__ == "__main__":
	main()
