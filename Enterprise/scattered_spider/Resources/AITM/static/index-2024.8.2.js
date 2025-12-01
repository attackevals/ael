const passwordFormHTML = `
<ak-form-static class="pf-c-form__group" user="{placeholderuser}" theme="light">
    <div class="form-control-static">
        <div class="avatar">
            <img class="pf-c-avatar" id="pf-c-avatar" src="{placeholderavatardata}" alt="User's avatar">
            {placeholderuser}
        </div>
        <div slot="link">
            <a href="/flows/cancel/">Not you?</a>
        </div>
    </div>
</ak-form-static>
<input id="pf-uid-field" name="username" autocomplete="username" type="hidden" value="{placeholderuser}">
<ak-flow-input-password required="" grab-focus="" class="pf-c-form__group" label="Password" invalid="false" prefill="">
    <ak-form-element required="" class="pf-c-form__group" label="Password" theme="light">
        <div class="pf-c-form__group">
            <label class="pf-c-form__label">
                <span class="pf-c-form__label-text">Password</span>
                <span class="pf-c-form__label-required" aria-hidden="true">*</span>
            </label>
        </div>
        <div class="pf-c-input-group">
            <input id="ak-stage-password-input" type="password" name="password" placeholder="Please enter your password" autofocus="" autocomplete="current-password" class="pf-c-form-control" required="" aria-invalid="false">
        </div>
    </ak-form-element>
</ak-flow-input-password>

<div class="pf-c-form__group pf-m-action">
    <button type="submit" class="pf-c-button pf-m-primary pf-m-block">
        Continue
    </button>
</div>
`;

const passwordFormFailureHTML = `
<ak-form-static class="pf-c-form__group" user="{placeholderuser}" theme="light">
    <div class="form-control-static">
        <div class="avatar">
            <img class="pf-c-avatar" id="pf-c-avatar" src="{placeholderavatardata}" alt="User's avatar">
            {placeholderuser}
        </div>
        <div slot="link">
            <a href="/flows/cancel/">Not you?</a>
        </div>
    </div>
</ak-form-static>
<input id="pf-uid-field" name="username" autocomplete="username" type="hidden" value="{placeholderuser}">
<ak-flow-input-password required="" grab-focus="" class="pf-c-form__group" label="Password" invalid="false" prefill="">
    <ak-form-element required="" class="pf-c-form__group" label="Password" theme="light">
        <div class="pf-c-form__group">
            <label class="pf-c-form__label">
                <span class="pf-c-form__label-text">Password</span>
                <span class="pf-c-form__label-required" aria-hidden="true">*</span>
            </label>
        </div>
        <div class="pf-c-input-group">
            <input id="ak-stage-password-input" type="password" name="password" placeholder="Please enter your password" autofocus="" autocomplete="current-password" class="pf-c-form-control" required="" aria-invalid="false">
        </div>
        <p class="pf-c-form__helper-text pf-m-error">
            <span class="pf-c-form__helper-text-icon">
                <i class="fas fa-exclamation-circle" aria-hidden="true"></i>
            </span>
            {MESSAGE PLACEHOLDER}
        </p>
    </ak-form-element>
</ak-flow-input-password>

<div class="pf-c-form__group pf-m-action">
    <button type="submit" class="pf-c-button pf-m-primary pf-m-block">
        Continue
    </button>
</div>
`;

const otpFormHTML = `
<ak-form-static class="pf-c-form__group" user="{placeholderuser}" theme="light">
    <div class="form-control-static">
        <div class="avatar">
            <img class="pf-c-avatar" id="pf-c-avatar" src="{placeholderavatardata}" alt="User's avatar">
            {placeholderuser}
        </div>
        <div slot="link">
            <a href="/flows/cancel/">Not you?</a>
        </div>
    </div>
</ak-form-static>
<input id="pf-uid-field" name="username" autocomplete="username" type="hidden" value="{placeholderuser}">
<div class="icon-description">
    <p>
        Open your two-factor authenticator app to view your authentication code.
    </p>
</div>
<ak-form-element required="" class="pf-c-form__group" label="Authentication code" theme="light">
    <div class="pf-c-form__group">
        <label class="pf-c-form__label">
            <span class="pf-c-form__label-text">Authentication code</span>
            <span class="pf-c-form__label-required" aria-hidden="true">*</span>
        </label>
        <slot></slot>
    </div>
    <input id="pf-otp-field" type="text" name="code" autofocus="" autocomplete="one-time-code" class="pf-c-form-control" required="" aria-invalid="false" inputmode="numeric" pattern="[0-9]*" placeholder="Please enter your code" value="">
</ak-form-element>

<div class="pf-c-form__group pf-m-action">
    <button type="submit" class="pf-c-button pf-m-primary pf-m-block">
        Continue
    </button>
</div>
`;

const otpFormFailureHTML = `
<ak-form-static class="pf-c-form__group" user="{placeholderuser}" theme="light">
    <div class="form-control-static">
        <div class="avatar">
            <img class="pf-c-avatar" id="pf-c-avatar" src="{placeholderavatardata}" alt="User's avatar">
            {placeholderuser}
        </div>
        <div slot="link">
            <a href="/flows/cancel/">Not you?</a>
        </div>
    </div>
</ak-form-static>
<input id="pf-uid-field" name="username" autocomplete="username" type="hidden" value="{placeholderuser}">
<div class="icon-description">
    <p>
        Open your two-factor authenticator app to view your authentication code.
    </p>
</div>
<ak-form-element required="" class="pf-c-form__group" label="Authentication code" theme="light">
    <div class="pf-c-form__group">
        <label class="pf-c-form__label">
            <span class="pf-c-form__label-text">Authentication code</span>
            <span class="pf-c-form__label-required" aria-hidden="true">*</span>
        </label>
        <slot></slot>
    </div>
    <input id="pf-otp-field" type="text" name="code" autofocus="" autocomplete="one-time-code" class="pf-c-form-control" required="" aria-invalid="false" inputmode="numeric" pattern="[0-9]*" placeholder="Please enter your code" value="">
    <p class="pf-c-form__helper-text pf-m-error">
        <span class="pf-c-form__helper-text-icon">
            <i class="fas fa-exclamation-circle" aria-hidden="true"></i>
        </span>
        {MESSAGE PLACEHOLDER}
    </p>
</ak-form-element>

<div class="pf-c-form__group pf-m-action">
    <button type="submit" class="pf-c-button pf-m-primary pf-m-block">
        Continue
    </button>
</div>
`;

const successPageHTML = `
<div>
    <p>Login successfully validated! Please close your browser for security purposes.</p>
</div>
`;

var form = document.getElementById("pf-c-login-form");

function handleLoginForm(event) {
    event.preventDefault();

    // Get username and password/token from login form, if provided
    var usernameElem = document.getElementById("pf-uid-field");
    var passwordElem = document.getElementById("ak-stage-password-input");
    var otpElem = document.getElementById("pf-otp-field");
    var username = usernameElem == null ? "" : usernameElem.value;
    var password = passwordElem == null ? "" : passwordElem.value;
    var otpCode = otpElem == null ? "" : otpElem.value;
    var avatarDataElem = document.getElementById("pf-c-avatar");
    var avatarData = avatarDataElem == null ? "" : avatarDataElem.src;

    // Login flow - submit username, then username+password, then username+token
    if (password == "") {
        if (otpCode == "") {
            // Phase 1 - submit username only
            (async () => {
                try {
                    const resp = await fetch('/api/v3/flows/executor/default-authentication-flow', {
                        method: 'POST',
                        headers: {
                            'Accept': 'application/json',
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({'component': 'ak-stage-identification', 'uid_field': username})
                    });

                    if (resp.status != 200) {
                        console.error(`Non-successful verification response HTTP status code from server: ${resp.status}`);
                    } else {
                        // Advance user to password submission form
                        var respJson = await resp.json();
                        avatarData = respJson['pending_user_avatar'];
                        form.innerHTML = passwordFormHTML.replace(/{placeholderuser}/g, username).replace(/{placeholderavatardata}/g, avatarData);
                    }
                } catch (error) {
                    var errorMsg = `Failure during login attempt: ${error}`;
                    console.error(errorMsg);
                }
            })();
        } else {
            // Phase 3 - validate MFA token
            (async () => {
                try {
                    const resp = await fetch('/api/v3/flows/executor/default-authentication-flow', {
                        method: 'POST',
                        headers: {
                            'Accept': 'application/json',
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({'component': 'ak-stage-authenticator-validate', 'uid_field': username, 'code': otpCode})
                    });

                    if (resp.status == 401) {
                        // Incorrect token
                        form.innerHTML = otpFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                           .replace(/{MESSAGE PLACEHOLDER}/g, 'Invalid Token. Please ensure the time on your device is accurate and try again.')
                                                           .replace(/{placeholderavatardata}/g, avatarData);
                    } else if (resp.status != 200) {
                        // Some other failure response
                        var failureMessage = `Non-successful verification response HTTP status code from server: ${resp.status}`;
                        form.innerHTML = otpFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                           .replace(/{MESSAGE PLACEHOLDER}/g, failureMessage)
                                                           .replace(/{placeholderavatardata}/g, avatarData);;
                    } else {
                        // Advance user to success page
                        console.info("Success!");
                        form.innerHTML = successPageHTML;
                    }
                } catch (error) {
                    var errorMsg = `Failure during OTP attempt: ${error}`;
                    console.error(errorMsg);
                    form.innerHTML = otpFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                       .replace(/{MESSAGE PLACEHOLDER}/g, errorMsg)
                                                       .replace(/{placeholderavatardata}/g, avatarData);
                }
            })();
        }
    } else {
        // Phase 2 - Validate password
        (async () => {
            try {
                const resp = await fetch('/api/v3/flows/executor/default-authentication-flow', {
                    method: 'POST',
                    headers: {
                        'Accept': 'application/json',
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({'component': 'ak-stage-password', 'uid_field': username, 'password': password})
                });

                if (resp.status == 401) {
                    // Incorrect creds
                    form.innerHTML = passwordFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                            .replace(/{MESSAGE PLACEHOLDER}/g, 'Invalid credentials')
                                                            .replace(/{placeholderavatardata}/g, avatarData);
                } else if (resp.status != 200) {
                    // Some other failure response
                    var failureMessage = `Non-successful verification response HTTP status code from server: ${resp.status}`;
                    form.innerHTML = passwordFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                            .replace(/{MESSAGE PLACEHOLDER}/g, failureMessage)
                                                            .replace(/{placeholderavatardata}/g, avatarData);
                } else {
                    // Advance user to MFA form or to success page
                    var respJson = await resp.json();
                    if (respJson.hasOwnProperty('component') && respJson['component'] == 'xak-flow-redirect') {
                        // Advance user to success page
                        console.info("Success!");
                        form.innerHTML = successPageHTML;
                    } else {
                        form.innerHTML = otpFormHTML.replace(/{placeholderuser}/g, username).replace(/{placeholderavatardata}/g, avatarData);
                    }
                }
            } catch (error) {
                var errorMsg = `Failure during login attempt: ${error}`;
                console.error(errorMsg);
                form.innerHTML = passwordFormFailureHTML.replace(/{placeholderuser}/g, username)
                                                        .replace(/{MESSAGE PLACEHOLDER}/g, errorMsg)
                                                        .replace(/{placeholderavatardata}/g, avatarData);
            }
        })();
    }
}

form.addEventListener('submit', handleLoginForm);
