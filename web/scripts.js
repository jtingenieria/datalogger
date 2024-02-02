document.addEventListener('DOMContentLoaded', function () {


    // Function to create a string configuration section
    function createStringConfig(index) {
        const section = document.createElement("section");
        section.classList.add("stringConfig");
        section.id = `string${index}`;
        section.innerHTML = `
            <h2>String ${index} Configuration</h2>
            <div class="sensorType">
                <label for="sensorTypeSelect${index}">Sensor Type:</label>
                <select class="sensorTypeSelect" id="sensorTypeSelect${index}">
                    <option value="DS18B20">DS18B20</option>
                    <option value="serial">Serial</option>
                </select>
            </div>
            <div class="quantityOfSignals">
                <label for="qtySignals${index}">Quantity of Signals:</label>
                <input type="number" class="qtySignals" id="qtySignals${index}" min="0" value="0"/>
            </div>
            <div class="deviceAddresses">
                <!-- Will be dynamically populated based on sensor type selection -->
            </div>
            <div class="serialConfig" style="display:none;">
                <!-- Will be shown/hidden based on sensor type selection -->
                <label for="separationChar${index}">Separation Character:</label>
                <input type="text" class="separationChar" id="separationChar${index}" />
                <label for="endChar${index}">End of Message Character:</label>
                <input type="text" class="endChar" id="endChar${index}" />
            </div>
        `;
        return section;
    }

    const configContainer = document.getElementById("configContainer");
    const dummy_node = document.getElementById("dummy_node");

    // Create string configuration sections
    for (let i = 1; i <= 6; i++) {
        const stringConfig = createStringConfig(i);
        configContainer.insertBefore(
            stringConfig,
            dummy_node
        );
    }

    // Set initial state to DS18B20 and trigger change event
    setInitialState();

    // Add event listeners
    addEventListeners();

    // Function to set initial state to DS18B20 and trigger change event
    function setInitialState() {
        const initialSensorTypeSelect = document.querySelector('.sensorTypeSelect');
        initialSensorTypeSelect.value = 'DS18B20';
        initialSensorTypeSelect.dispatchEvent(new Event('change'));
    }

    // Function to create output order selects
    function createOutputOrderSelects() {
        const outputOrderContainer = document.getElementById('outputOrder');
        for (let i = 1; i <= 6; i++) {
            const label = document.createElement('label');
            label.textContent = `Output order of String ${i}:`;
            outputOrderContainer.appendChild(label);
            const select = document.createElement('select');
            select.id = `outputOrderSelect${i}`;
            for (let j = 1; j <= 6; j++) {
                const option = document.createElement('option');
                option.value = j;
                option.textContent = j;
                select.appendChild(option);
            }
            const defaultOption = document.createElement('option');
            defaultOption.value = '-1';
            defaultOption.textContent = 'Disabled';
            defaultOption.selected = true;
            select.appendChild(defaultOption);
            outputOrderContainer.appendChild(select);
        }
    }

    createOutputOrderSelects();

    // Function to add event listeners
    function addEventListeners() {
        // Save button click event listener
        const saveButton = document.getElementById('saveButton');
        saveButton.addEventListener('click', saveParameters);

        // Import button click event listener
        const importButton = document.getElementById('importButton');
        importButton.addEventListener('click', importParameters);

        // Change event listeners for sensor type and quantity of signals
        document.addEventListener('change', handleSensorTypeChange);
        document.addEventListener('input', handleQuantityOfSignalsChange);
        document.addEventListener('change', handleOutputOrderChange);
    }

    // Function to handle changes in sensor type
    function handleSensorTypeChange(event) {
        const target = event.target;
        if (target && target.classList.contains('sensorTypeSelect')) {
            const parentSection = target.closest('.stringConfig');
            const sensorType = target.value;
            const quantityOfSignals = parentSection.querySelector('.quantityOfSignals');
            const deviceAddresses = parentSection.querySelector('.deviceAddresses');
            const serialConfig = parentSection.querySelector('.serialConfig');

            quantityOfSignals.style.display = (sensorType === 'DS18B20') ? 'block' : 'none';
            deviceAddresses.style.display = (sensorType === 'DS18B20') ? 'block' : 'none';
            serialConfig.style.display = (sensorType === 'serial') ? 'block' : 'none';
        }
    }

    // Function to handle changes in quantity of signals
    function handleQuantityOfSignalsChange(event) {
        const target = event.target;
        if (target && target.classList.contains('qtySignals')) {
            const parentSection = target.closest('.stringConfig');
            const deviceAddresses = parentSection.querySelector('.deviceAddresses');
            const qtySignals = parseInt(target.value);
            if (!isNaN(qtySignals)) {
                addDeviceAddressInputs(parentSection, qtySignals);
            }
        }
    }

    // Function to add device address inputs dynamically
    function addDeviceAddressInputs(parentSection, qtySignals) {
        const deviceAddresses = parentSection.querySelector('.deviceAddresses');
        const currentQtySignals = deviceAddresses.querySelectorAll('input').length;
        console.log(deviceAddresses.querySelectorAll('input').length);
        if (qtySignals > currentQtySignals) {

            for (let i = currentQtySignals + 1; i <= qtySignals; i++) {
                const label = document.createElement('label');
                label.textContent = `Sensor ${i}:`;
                const input = document.createElement('input');
                input.type = 'text';
                input.id = `${parentSection.id}_s${i}`;
                deviceAddresses.appendChild(label);
                deviceAddresses.appendChild(input);
            }
        } else {
            const labels = deviceAddresses.querySelectorAll('label');
            const inputs = deviceAddresses.querySelectorAll('input');
            for (let i = qtySignals; i < labels.length; i++) {
                deviceAddresses.removeChild(labels[i]);
                deviceAddresses.removeChild(inputs[i]);
            }
        }
    }

    // Function to handle changes in output order
    function handleOutputOrderChange(event) {
        const target = event.target;
        if (target && target.id.startsWith('outputOrderSelect')) {
            const selectedValue = parseInt(target.value);
            if (selectedValue >= 1 && selectedValue <= 6) {
                // Check if this value is already selected in any other select
                const selects = document.querySelectorAll('[id^="outputOrderSelect"]');
                let count = 0;
                selects.forEach(function (select) {
                    if (parseInt(select.value) === selectedValue) {
                        count++;
                    }
                });
                if (count > 1) {
                    // Reset this select to -1
                    target.value = '-1';
                }
            }
        }
    }

    // Function to save parameters as JSON file
    function saveParameters() {
        const parameters = collectParameters();
        const jsonData = JSON.stringify(parameters, null, 2);
        const blob = new Blob([jsonData], { type: 'text/plain' });
        const downloadLink = createDownloadLink(blob, 'parameters.json');
        downloadLink.click();
    }

    // Function to collect parameters from the configuration
    function collectParameters() {
        const parameters = {};

        // Collect string configurations
        const stringConfigs = document.querySelectorAll('.stringConfig');
        stringConfigs.forEach((section, index) => {
            const config = {};
            const sensorTypeSelect = section.querySelector('.sensorTypeSelect');
            config.SensorType = sensorTypeSelect.value;
            const quantityOfSignalsInput = section.querySelector('.qtySignals');
            config.QuantityOfSignals = quantityOfSignalsInput.value;
            if (config.SensorType === 'DS18B20') {
                const deviceAddressesInputs = section.querySelectorAll('.deviceAddresses input');
                config.DeviceAddresses = Array.from(deviceAddressesInputs).map(input => input.value);
            } else {
                const separationCharInput = section.querySelector('.separationChar');
                config.SeparationCharacter = separationCharInput.value;
                const endCharInput = section.querySelector('.endChar');
                config.EndOfMessageCharacter = endCharInput.value;
            }
            parameters[`String${index + 1}`] = config;
        });

        // Collect output format configuration
        const outputOrderSelects = document.querySelectorAll('[id^="outputOrderSelect"]');
        parameters.OutputFormat = Array.from(outputOrderSelects).map(select => select.value);

        // Collect output string format
        const formatStringInput = document.getElementById('formatString');
        parameters.OutputStringFormat = formatStringInput.value;

        return parameters;
    }

    // Function to create download link
    function createDownloadLink(blob, filename) {
        const downloadLink = document.createElement('a');
        downloadLink.href = URL.createObjectURL(blob);
        downloadLink.download = filename;
        return downloadLink;
    }

    // Function to import parameters from JSON file
    function importParameters() {
        const fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.accept = '.json';
        fileInput.click();
        fileInput.addEventListener('change', handleFileSelection);
    }

    // Function to handle file selection
    function handleFileSelection(event) {
        const file = event.target.files[0];
        const reader = new FileReader();
        reader.onload = handleFileRead;
        reader.readAsText(file);
    }

    // Function to handle file read
    function handleFileRead(event) {
        const importedData = JSON.parse(event.target.result);
        updateConfiguration(importedData);
    }

    // Function to update configuration with imported data
    function updateConfiguration(importedData) {
        updateStringConfigurations(importedData);
        updateOutputFormat(importedData);
        updateOutputStringFormat(importedData);
    }

    // Function to update string configurations
    function updateStringConfigurations(importedData) {
        const stringConfigs = document.querySelectorAll('.stringConfig');
        stringConfigs.forEach((section, index) => {
            const config = importedData[`String${index + 1}`];
            if (config) {
                const sensorTypeSelect = section.querySelector('.sensorTypeSelect');
                sensorTypeSelect.value = config.SensorType;
                if (config.SensorType === 'DS18B20') {
                    const quantityOfSignalsInput = section.querySelector('.qtySignals');
                    quantityOfSignalsInput.value = config.QuantityOfSignals;
                    updateDeviceAddresses(section, config.DeviceAddresses);
                } else {
                    const separationCharInput = section.querySelector('.separationChar');
                    separationCharInput.value = config.SeparationCharacter;
                    const endCharInput = section.querySelector('.endChar');
                    endCharInput.value = config.EndOfMessageCharacter;
                }
            }
        });
    }

    // Function to update device addresses
    function updateDeviceAddresses(section, addresses) {
        addDeviceAddressInputs(section, addresses.length)
        const deviceAddressesInputs = section.querySelectorAll('.deviceAddresses input');
        addresses.forEach((address, i) => {
            deviceAddressesInputs[i].value = address;
        });
    }

    // Function to update output format
    function updateOutputFormat(importedData) {
        const outputOrderSelects = document.querySelectorAll('[id^="outputOrderSelect"]');
        importedData.OutputFormat.forEach((value, i) => {
            outputOrderSelects[i].value = value;
        });
    }

    // Function to update output string format
    function updateOutputStringFormat(importedData) {
        const formatStringInput = document.getElementById('formatString');
        formatStringInput.value = importedData.OutputStringFormat;
    }
});