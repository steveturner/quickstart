import com.android.build.api.variant.BuildConfigField
import java.io.FileInputStream
import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.jetbrains.kotlin.android)
    alias(libs.plugins.compose.compiler)
    alias(libs.plugins.google.services)
}

fun loadEnvProperties(): Properties {
    val properties = Properties()
    val envFile = rootProject.file("../../.env")

    if (envFile.exists()) {
        FileInputStream(envFile).use { properties.load(it) }
    } else {
        val requiredEnvVars = listOf(
            "DITTO_APP_ID",
            "DITTO_PLAYGROUND_TOKEN",
            "DITTO_AUTH_URL",
            "DITTO_WEBSOCKET_URL"
        )

        for (envVar in requiredEnvVars) {
            val value = System.getenv(envVar)
                ?: throw RuntimeException("Required environment variable $envVar not found")
            properties[envVar] = value
        }
    }
    return properties
}

androidComponents {
    onVariants {
        val prop = loadEnvProperties()
        fun propVal(key: String) = prop[key].toString().trim('"')
        it.buildConfigFields.put("DITTO_APP_ID",
            BuildConfigField("String", "\"${propVal("DITTO_APP_ID")}\"", "Ditto app ID"))
        it.buildConfigFields.put("DITTO_PLAYGROUND_TOKEN",
            BuildConfigField("String", "\"${propVal("DITTO_PLAYGROUND_TOKEN")}\"", "Ditto token"))
        it.buildConfigFields.put("DITTO_AUTH_URL",
            BuildConfigField("String", "\"${propVal("DITTO_AUTH_URL")}\"", "Ditto auth URL"))
        it.buildConfigFields.put("DITTO_WEBSOCKET_URL",
            BuildConfigField("String", "\"${propVal("DITTO_WEBSOCKET_URL")}\"", "Ditto WS URL"))
    }
}

android {
    namespace = "live.ditto.pubsec.pos"
    compileSdk = 35

    defaultConfig {
        applicationId = "live.ditto.pubsec.pos"
        minSdk = 23
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        vectorDrawables { useSupportLibrary = true }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    kotlinOptions { jvmTarget = "1.8" }

    buildFeatures {
        buildConfig = true
        compose = true
    }

    packaging {
        resources { excludes += "/META-INF/{AL2.0,LGPL2.1}" }
    }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.activity.compose)
    implementation(libs.androidx.appcompat)

    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)
    implementation(libs.androidx.navigation.compose)
    implementation(libs.androidx.runtime.livedata)

    implementation(platform(libs.koin.bom))
    implementation(libs.koin.core)
    implementation(libs.koin.android)
    implementation(libs.koin.androidx.compose)
    implementation(libs.koin.androidx.compose.navigation)

    implementation(libs.live.ditto)

    implementation(platform(libs.firebase.bom))
    implementation(libs.firebase.firestore)
    implementation(libs.kotlinx.coroutines.play.services)

    testImplementation(libs.junit)
    testImplementation(libs.kotlinx.coroutines)
    testImplementation("io.insert-koin:koin-test:4.1.0")
    testImplementation("io.insert-koin:koin-test-junit4:4.1.0")
    testImplementation("io.mockk:mockk:1.13.12")

    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.ui.test.junit4)

    debugImplementation(libs.androidx.ui.tooling)
    debugImplementation(libs.androidx.ui.test.manifest)
}
