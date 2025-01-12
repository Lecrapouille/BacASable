*** Settings ***
Library           SeleniumLibrary

*** Variables ***
${BROWSER}        Chrome
${URL}            https://www.google.com
${SEARCH_TERM}    GPT-4

*** Test Cases ***
Test Search Page
    Open Browser    url=${URL}    browser=${BROWSER}
    Set Browser Implicit Wait    4
    Handle Consent
    Input Text    name=q    ${SEARCH_TERM}
    Press Key    name=q    \\13
    Wait Until Page Contains    ${SEARCH_TERM}
    Click First Result

*** Keywords ***
Handle Consent
    # Exemple de clic sur un bouton de consentement, ajustez le sélecteur selon le site
    Click Button    id=L2AGLb
    Set Browser Implicit Wait    4

Click First Result
    # Cliquez sur le premier lien des résultats de recherche
    Click Element    xpath=(//h3)[1]