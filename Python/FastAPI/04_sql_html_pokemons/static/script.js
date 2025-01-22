// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Pokemon API client-side JavaScript
// //////////////////////////////////////////////////////////////////////////////////////////////////

// DOM Elements
const pokemonGrid = document.getElementById('pokemonGrid');
const addPokemonForm = document.getElementById('addPokemonForm');
const initDbButton = document.getElementById('initDb');
const clearDbButton = document.getElementById('clearDb');
const typeFilter = document.getElementById('typeFilter');
const powerFilter = document.getElementById('powerFilter');
const applyFiltersButton = document.getElementById('applyFilters');
const showAllPokemonsButton = document.getElementById('showAllPokemons');

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Create a Pokemon card element
// /// @param[in] pokemon Pokemon data object
// /// @return HTML element representing the Pokemon card
// //////////////////////////////////////////////////////////////////////////////////////////////////
function createPokemonCard(pokemon) {
    const card = document.createElement('div');
    card.className = 'pokemon-card';
    card.innerHTML = `
        <h3>${pokemon.name}</h3>
        <div class="type">${pokemon.type}</div>
        <div class="power">Puissance: ${pokemon.power}</div>
        <div class="actions">
            <button onclick="deletePokemon(${pokemon.id})">Supprimer</button>
            <button onclick="showEditForm(${pokemon.id})">Modifier</button>
        </div>
    `;
    return card;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Load all Pokemons with optional filters
// /// @param[in] type Optional type filter
// /// @param[in] minPower Optional minimum power filter
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function loadPokemons(type = '', minPower = '') {
    try {
        let url = '/pokemons/';
        const params = new URLSearchParams();
        if (type) params.append('type', type);
        if (minPower) params.append('min_power', minPower);
        if (params.toString()) url += '?' + params.toString();

        const response = await fetch(url);
        const pokemons = await response.json();
        
        pokemonGrid.innerHTML = '';
        pokemons.forEach(pokemon => {
            pokemonGrid.appendChild(createPokemonCard(pokemon));
        });
    } catch (error) {
        console.error('Erreur lors du chargement des Pokémons:', error);
        alert('Erreur lors du chargement des Pokémons');
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Add a new Pokemon
// /// @param[in] event Form submit event
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function addPokemon(event) {
    event.preventDefault();
    
    const pokemonData = {
        name: document.getElementById('pokemonName').value,
        type: document.getElementById('pokemonType').value,
        power: parseInt(document.getElementById('pokemonPower').value)
    };

    try {
        const response = await fetch('/pokemons/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(pokemonData)
        });

        if (response.ok) {
            addPokemonForm.reset();
            loadPokemons();
        } else {
            throw new Error('Erreur lors de l\'ajout du Pokémon');
        }
    } catch (error) {
        console.error('Erreur:', error);
        alert(error.message);
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Delete a Pokemon
// /// @param[in] id Pokemon ID to delete
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function deletePokemon(id) {
    if (!confirm('Êtes-vous sûr de vouloir supprimer ce Pokémon ?')) return;

    try {
        const response = await fetch(`/pokemons/${id}`, {
            method: 'DELETE'
        });

        if (response.ok) {
            loadPokemons();
        } else {
            throw new Error('Erreur lors de la suppression du Pokémon');
        }
    } catch (error) {
        console.error('Erreur:', error);
        alert(error.message);
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Show edit form for a Pokemon
// /// @param[in] id Pokemon ID to edit
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function showEditForm(id) {
    const newName = prompt('Nouveau nom:');
    const newType = prompt('Nouveau type:');
    const newPower = prompt('Nouvelle puissance:');

    if (!newName && !newType && !newPower) return;

    const updateData = {};
    if (newName) updateData.name = newName;
    if (newType) updateData.type = newType;
    if (newPower) updateData.power = parseInt(newPower);

    try {
        const response = await fetch(`/pokemons/${id}`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(updateData)
        });

        if (response.ok) {
            loadPokemons();
        } else {
            throw new Error('Erreur lors de la modification du Pokémon');
        }
    } catch (error) {
        console.error('Erreur:', error);
        alert(error.message);
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Initialize database with default Pokemons
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function initializeDatabase() {
    try {
        const response = await fetch('/init-db/', {
            method: 'POST'
        });

        if (response.ok) {
            loadPokemons();
            alert('Base de données initialisée avec succès !');
        } else {
            throw new Error('Erreur lors de l\'initialisation de la base de données');
        }
    } catch (error) {
        console.error('Erreur:', error);
        alert(error.message);
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// /// @brief Clear the entire database
// //////////////////////////////////////////////////////////////////////////////////////////////////
async function clearDatabase() {
    if (!confirm('Êtes-vous sûr de vouloir vider la base de données ?')) return;

    try {
        const response = await fetch('/clear-db/', {
            method: 'POST'
        });

        if (response.ok) {
            loadPokemons();
            alert('Base de données vidée avec succès !');
        } else {
            throw new Error('Erreur lors du vidage de la base de données');
        }
    } catch (error) {
        console.error('Erreur:', error);
        alert(error.message);
    }
}

// Event Listeners
addPokemonForm.addEventListener('submit', addPokemon);
initDbButton.addEventListener('click', initializeDatabase);
clearDbButton.addEventListener('click', clearDatabase);
applyFiltersButton.addEventListener('click', () => {
    loadPokemons(typeFilter.value, powerFilter.value);
});
showAllPokemonsButton.addEventListener('click', () => {
    typeFilter.value = '';
    powerFilter.value = '';
    loadPokemons();
});

// Initial load
loadPokemons(); 